#include "json_reader.h"
#include "json_builder.h"

#include <sstream> 

using namespace std::literals;

const std::set<std::string> JsonReader::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.GetStop(stop_name)->buses;
}

bool JsonReader::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.GetBus(bus_number);
}

bool JsonReader::IsStopName(const std::string_view stop_name) const {
    return catalogue_.GetStop(stop_name);
}

svg::Document JsonReader::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}

const json::Node& JsonReader::GetBaseRequests() const {
    if (!input_.GetRoot().AsMap().count("base_requests")) {
        return dummy_;    
    } 
    
    return input_.GetRoot().AsMap().at("base_requests");
}

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsMap().count("stat_requests")) {
        return dummy_;
    }
    
    return input_.GetRoot().AsMap().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsMap().count("render_settings")) {
        return dummy_;    
    } 
    
    return input_.GetRoot().AsMap().at("render_settings");
}

void JsonReader::ProcessRequests() const {
    const auto& stat_requests = GetStatRequests();
    
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        
        if (type == "Stop") {
            result.push_back(PrintStop(request_map).AsMap());
        } 
        
        if (type == "Bus") {
            result.push_back(PrintRoute(request_map).AsMap());    
        }
        
        if (type == "Map") {
            result.push_back(PrintMap(request_map).AsMap());    
        } 
    }

    json::Print(json::Document{ result }, std::cout);
}

void JsonReader::FillStopDistances() const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops: arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue_.GetStop(stop_name);
                auto to = catalogue_.GetStop(to_name);
                catalogue_.SetDistance(from, to, dist);
            }
        }
    }
}

JsonReader::BusData JsonReader::FillRoute(const json::Dict& request_map) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const trasport_catalogue::Stop*> stops;
    
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue_.GetStop(stop.AsString()));
    }
    
    bool circular_route = request_map.at("is_roundtrip").AsBool();
    return { bus_number, stops, circular_route };
}

void JsonReader::FillCatalogue() {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            catalogue_.AddStop(stop_name, coordinates);
        }
    }
    
    FillStopDistances();
    
    for (auto& request_bus : arr) {
        const auto& request_bus_map = request_bus.AsMap();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = FillRoute(request_bus_map);
            catalogue_.AddBus(bus_number, stops, circular_route);
        }
    }
}

JsonReader::StopData JsonReader::FillStop(const json::Dict& request_map) const {
    std::string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = request_map.at("road_distances").AsMap();
    
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    
    return { stop_name, coordinates, stop_distances };
}

svg::Color ConvertColorToRgbOrRgbaFormat(std::vector<json::Node> color_array) {
    svg::Color output_color;
    if (color_array.size() == 3) {
        svg::Rgb rgb_color = { static_cast<uint8_t>(color_array.begin()->AsInt()),
					           static_cast<uint8_t>((color_array.begin() + 1)->AsInt()),
							   static_cast<uint8_t>((color_array.begin() + 2)->AsInt()) };
		output_color = rgb_color;
	} else if (color_array.size() == 4) {
        svg::Rgba rgba_color = { static_cast<uint8_t>(color_array.begin()->AsInt()),
								 static_cast<uint8_t>((color_array.begin() + 1)->AsInt()),
								 static_cast<uint8_t>((color_array.begin() + 2)->AsInt()),
			                     (color_array.begin() + 3)->AsDouble() };
		output_color = rgba_color;
	}
    
	return output_color;
}

 renderer::MapRenderer JsonReader::FillRenderSettings() {
    FillCatalogue();
	renderer::RenderSettings output_settings;
	std::string render_settings = "render_settings"s;
    
    if (input_.GetRoot().AsMap().count(render_settings) <= 0) {
        return output_settings;
    }
    
	auto& data = input_.GetRoot().AsMap().at(render_settings).AsMap();
	double width = data.at( "width"s ).AsDouble();
	double height = data.at( "height"s ).AsDouble();
	double padding = data.at( "padding"s ).AsDouble();
	double line_width = data.at( "line_width"s ).AsDouble();
	double stop_radius = data.at( "stop_radius"s ).AsDouble();
	int	bus_label_font_size = data.at( "bus_label_font_size"s ).AsInt();
	auto bus_label_offset_array = data.at( "bus_label_offset"s ).AsArray();
	svg::Point bus_label_offset = { bus_label_offset_array.front().AsDouble(), bus_label_offset_array.back().AsDouble() };
	int	stop_label_font_size = data.at( "stop_label_font_size"s ).AsInt();
	auto stop_label_offset_array = data.at( "stop_label_offset"s ).AsArray();
	svg::Point stop_label_offset = { stop_label_offset_array.front().AsDouble(), stop_label_offset_array.back().AsDouble() };
            
	svg::Color underlayer_color;        
	if (data.at( "underlayer_color"s ).IsArray()) {
		underlayer_color = ConvertColorToRgbOrRgbaFormat(data.at( "underlayer_color"s ).AsArray());
	} else if (data.at( "underlayer_color"s ).IsString()) {
		underlayer_color = data.at( "underlayer_color"s ).AsString();
	}
            
	double underlayer_width = data.at( "underlayer_width"s ).AsDouble();
	std::vector<svg::Color> color_palette;
	auto color_palette_array = data.at( "color_palette"s ).AsArray();
            
	for (auto& value : color_palette_array) {
		if (value.IsString()) {
			color_palette.push_back(value.AsString());
		} else if (value.IsArray()) {
			color_palette.push_back(ConvertColorToRgbOrRgbaFormat(value.AsArray()));
		}
	}
            
	output_settings = { width, height, padding, stop_radius, line_width, bus_label_font_size, bus_label_offset, stop_label_font_size, stop_label_offset, underlayer_color, underlayer_width, color_palette };   
    
    return output_settings;
}

const json::Node JsonReader::PrintRoute(const json::Dict& request_map) const {
    json::Node result;
    const std::string& route_number = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
    
    if (IsBusNumber(route_number)) {
        const auto& route_info = catalogue_.GetRouteInformation(route_number);
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("curvature").Value(route_info->curvature)
                        .Key("route_length").Value(route_info->route_length)
                        .Key("stop_count").Value(static_cast<int>(route_info->stops_count))
                        .Key("unique_stop_count").Value(static_cast<int>(route_info->unique_stops_count))
                    .EndDict()
                .Build();
    } else {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    }
    
    return result;
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map) const {
    json::Node result;
    const std::string& stop_name = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
    
    if (!IsStopName(stop_name)) {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    } else {
        json::Array buses;
        for (const auto& bus : GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("buses").Value(buses)
                    .EndDict()
                .Build();
    }

    return result;
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = RenderMap();
    map.Render(strm);
    
    result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("map").Value(strm.str())
                .EndDict()
            .Build();

    return result;
}