#include "json_reader.h"


int main() {
    
    //std::cout << "Ready" << std::endl;                 // remove this line
    
    transport::TransportCatalogue catalogue;
    transport::renderer::MapRenderer renderer;
    transport::TransportRouter router(catalogue);
    transport::RequestHandler handler(catalogue, renderer);
    transport::io::JsonReader reader(catalogue, handler, renderer, router);
    reader.ReadJsonFromStream(std::cin);
    reader.FillDB();
    reader.ProcessAndApplyRenderSettings();
    reader.ProcessAndApplyRouterSettings();
    
    reader.ProcessStatRequests(std::cout);
    
}
