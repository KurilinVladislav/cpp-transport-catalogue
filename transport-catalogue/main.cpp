#include "json_reader.h"


int main() {
    
    transport::TransportCatalogue catalogue;
    transport::renderer::MapRenderer renderer;
    transport::RequestHandler handler(catalogue, renderer);
    transport::io::JsonReader reader(catalogue, handler, renderer);
    reader.ReadJsonFromStream(std::cin);
    reader.FillDB();
    reader.ProcessAndApplyRenderSettings();
    reader.ProcessStatRequests(std::cout);
    
}
