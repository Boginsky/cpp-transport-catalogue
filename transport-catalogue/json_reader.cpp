#include "json_reader.h"

using namespace std::literals;

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

void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        
        if (type == "Stop") {
            result.push_back(PrintStop(request_map, rh).AsMap());
        } 
        
        if (type == "Bus") {
            result.push_back(PrintRoute(request_map, rh).AsMap());    
        }
        
        if (type == "Map") {
            result.push_back(PrintMap(request_map, rh).AsMap());    
        } 
    }

    json::Print(json::Document{ result }, std::cout);
}

void JsonReader::FillCatalogue(trasport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    
    FillStopDistances(catalogue);
    
    for (auto& request_bus : arr) {
        const auto& request_bus_map = request_bus.AsMap();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = FillRoute(request_bus_map, catalogue);
            catalogue.AddBus(bus_number, stops, circular_route);
        }
    }
}

std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::FillStop(const json::Dict& request_map) const {
    std::string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = request_map.at("road_distances").AsMap();
    
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    
    return std::make_tuple(stop_name, coordinates, stop_distances);
}

void JsonReader::FillStopDistances(trasport_catalogue::TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops: arr) {
        const auto& request_stops_map = request_stops.AsMap();
        const auto& type = request_stops_map.at("type").AsString();
        
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue.GetStop(stop_name);
                auto to = catalogue.GetStop(to_name);
                catalogue.SetDistance(from, to, dist);
            }
        }
    }
}

std::tuple<std::string_view, std::vector<const trasport_catalogue::Stop*>, bool> JsonReader::FillRoute(const json::Dict& request_map, trasport_catalogue::TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const trasport_catalogue::Stop*> stops;
    
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.GetStop(stop.AsString()));
    }
    
    bool circular_route = request_map.at("is_roundtrip").AsBool();
    return std::make_tuple(bus_number, stops, circular_route);
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

renderer::MapRenderer JsonReader::FillRenderSettings() const {
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

const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& route_number = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    
    if (!rh.IsBusNumber(route_number)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    } else {
        result["curvature"] = rh.GetBusStat(route_number)->curvature;
        result["route_length"] = rh.GetBusStat(route_number)->route_length;
        result["stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->stops_count);
        result["unique_stop_count"] = static_cast<int>(rh.GetBusStat(route_number)->unique_stops_count);
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& stop_name = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    } else {
        json::Array buses;
        for (auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result["buses"] = buses;
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{ result };
}