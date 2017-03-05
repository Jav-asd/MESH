#include "setup.h"

int main(){
  // initializing material
  Ptr<FileLoader> fileLoader = FileLoader::instanceNew();
  fileLoader->load("GaAs.txt");
  Ptr<Material> GaAs = Material::instanceNew("GaAs", fileLoader->getOmegaList(), fileLoader->getEpsilonList(), fileLoader->getNumOfOmega());
  fileLoader->load("Vacuum.txt");
  Ptr<Material> Vacuum = Material::instanceNew("Vacuum", fileLoader->getOmegaList(), fileLoader->getEpsilonList(), fileLoader->getNumOfOmega());
  fileLoader->load("PEC.txt");
  Ptr<Material> PEC = Material::instanceNew("PEC", fileLoader->getOmegaList(), fileLoader->getEpsilonList(), fileLoader->getNumOfOmega());


  // initializing layer
  Ptr<Layer> PECLayer = Layer::instanceNew("PECLayer", PEC, 0);
  Ptr<Layer> GaAsLayer = Layer::instanceNew("GaAsLayer", GaAs, 1e-6);
  Ptr<Layer> VaccumLayer = Layer::instanceNew("VacuumLayer", Vacuum, 0);
  GaAsLayer->setIsSource();

  // initializing structure
  Ptr<Structure> structure = Structure::instanceNew();
  structure->addLayer(PECLayer);
  structure->addLayer(GaAsLayer);
  structure->addLayer(VaccumLayer);

  // initializing simulation
  Ptr<SimulationPlanar> s = SimulationPlanar::instanceNew();

  s->addStructure(structure);

  s->setTargetLayerByLayer(VaccumLayer);
  s->setKxIntegral(1.0);
  s->setOutputFile("test_output.txt");
  s->build();
  s->run();

  return 0;
}
