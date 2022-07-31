#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    transport::TransportCatalogue catalogue;
    transport::renderer::MapRenderer renderer;
    transport::TransportRouter router(catalogue);
    transport::RequestHandler handler(catalogue, renderer);
    transport::io::JsonReader reader(catalogue, handler, renderer, router);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // make base here
        reader.ReadJsonFromStream(std::cin);
        reader.FillDB();
        transport::Serializer serializer(catalogue, renderer, router, reader);
        reader.ProcessAndApplyRenderSettings();
        reader.ProcessAndApplyRouterSettings();
        router.Init();
        serializer.SaveData();

    } else if (mode == "process_requests"sv) {

        // process requests here
        reader.ReadJsonFromStream(std::cin);
        transport::Serializer serializer(catalogue, renderer, router, reader);
        serializer.LoadData();
        reader.ProcessStatRequests(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
