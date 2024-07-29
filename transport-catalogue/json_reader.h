#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

class JsonReader {
    public:
        JsonReader(std::istream& input, trasport_catalogue::TransportCatalogue& catalogue)
            : input_(json::Load(input)), catalogue_(catalogue), renderer_(FillRenderSettings()) {
        }

        const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;
        bool IsBusNumber(const std::string_view bus_number) const;
        bool IsStopName(const std::string_view stop_name) const;

        svg::Document RenderMap() const;
    
        const json::Node& GetBaseRequests() const;
        const json::Node& GetStatRequests() const;
        const json::Node& GetRenderSettings() const;

        void ProcessRequests(const json::Node& stat_requests) const;

        void FillCatalogue();
        renderer::MapRenderer FillRenderSettings();

        const json::Node PrintRoute(const json::Dict& request_map) const;
        const json::Node PrintStop(const json::Dict& request_map) const;
        const json::Node PrintMap(const json::Dict& request_map) const;
    private:
        json::Document input_;
        json::Node dummy_ = nullptr;
    
        trasport_catalogue::TransportCatalogue& catalogue_;
        renderer::MapRenderer renderer_;
    
        struct StopData {
            std::string_view name;
            geo::Coordinates coordinates;
            std::map<std::string_view, int> stop_distances;
        };
    
        struct BusData {
            std::string_view name;
            std::vector<const trasport_catalogue::Stop*> stops;
            bool is_circular;
        };
    
        StopData FillStop(const json::Dict& request_map) const;
        void FillStopDistances() const;
        BusData FillRoute(const json::Dict& request_map) const;
};