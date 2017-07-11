/* Copyright (C) 2016-2018, Stanford University
 * This file is part of MESH
 * Written by Kaifeng Chen (kfchen@stanford.edu)
 *
 * MESH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MESH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Mesh.h"

namespace MESH{
  /*==============================================*/
  // Constructor of the FileLoader class
  /*==============================================*/
  FileLoader::FileLoader(){
    omegaList_ = nullptr;
    epsilonList_.epsilonVals = nullptr;
  }
  /*==============================================*/
  // This is a thin wrapper for the usage of smart pointer
  /*==============================================*/
  Ptr<FileLoader> FileLoader::instanceNew(){
    return new FileLoader();
  }

  /*==============================================*/
  // Function checking a line contains more than 3 spaces
  /*==============================================*/
  EPSTYPE checkType(std::string line){
    int numOfSpace = 0;
    UTILITY::preprocessString(line);
    for(size_t i = 0; i < line.length(); i++){
      if(isspace(line.at(i))) numOfSpace++;
    }
    switch (numOfSpace) {
      case 2: return SCALAR_;
      case 6: return DIAGONAL_;
      case 10: return TENSOR_;
      default:{
        std::cerr << "Input type wrong: should be of 2, 6 or 10 tabs (spaces)!" << std::endl;
        throw UTILITY::UnknownTypeException("Input type wrong: should be of 2, 6 or 10 tabs (spaces)!");
      }
    }
    return SCALAR_;
  }
  /*==============================================*/
  // Function reads a file from disk
  // @args
  // fileName: the name of the file
  /*==============================================*/
  void FileLoader::load(const std::string fileName){
    std::ifstream inputFile(fileName);
    if(!inputFile.good()){
      std::cerr << fileName + " not exists!" << std::endl;
      throw UTILITY::FileNotExistException(fileName + " not exists!");
    }
    std::string line;
    int count = 0;
    EPSTYPE type = SCALAR_;
    while(std::getline(inputFile, line)){
      count++;
      type = checkType(line);
    }
    inputFile.close();

    if(!preSet_){
      numOfOmega_ = count;
      preSet_ = true;
      omegaList_ = new double[numOfOmega_];
      epsilonList_.epsilonVals = new EpsilonVal[numOfOmega_];
    }
    else{
      if(numOfOmega_ != count){
        std::cerr << "wrong omega points!" << std::endl;
        throw UTILITY::RangeException("wrong omega points!");
      }
    }

    epsilonList_.type_ = type;
    std::ifstream inputFile2(fileName);
    for(int i = 0; i < numOfOmega_; i++){
      inputFile2 >> omegaList_[i];
      if(epsilonList_.type_ == SCALAR_){
        double imag;
        inputFile2 >> epsilonList_.epsilonVals[i].scalar[0] >> imag;
        epsilonList_.epsilonVals[i].scalar[1] = -imag;
      }
      else if(epsilonList_.type_ == DIAGONAL_){
        double real1, imag1, real2, imag2, real3, imag3;
        inputFile2 >> real1 >> imag1 >> real2 >> imag2 >> real3 >> imag3;
        epsilonList_.epsilonVals[i].diagonal[0] = real1;
        epsilonList_.epsilonVals[i].diagonal[1] = -imag1;
        epsilonList_.epsilonVals[i].diagonal[2] = real2;
        epsilonList_.epsilonVals[i].diagonal[3] = -imag2;
        epsilonList_.epsilonVals[i].diagonal[4] = real3;
        epsilonList_.epsilonVals[i].diagonal[5] = -imag3;
      }
      else{
        double real1, imag1, real2, imag2, real3, imag3, real4, imag4, real5, imag5;
        inputFile2 >> real1 >> imag1 >> real2 >> imag2 >> real3 >> imag3 >> real4 >> imag4 >> real5 >> imag5;
        epsilonList_.epsilonVals[i].tensor[0] = real1;
        epsilonList_.epsilonVals[i].tensor[1] = -imag1;
        epsilonList_.epsilonVals[i].tensor[2] = real2;
        epsilonList_.epsilonVals[i].tensor[3] = -imag2;
        epsilonList_.epsilonVals[i].tensor[4] = real3;
        epsilonList_.epsilonVals[i].tensor[5] = -imag3;
        epsilonList_.epsilonVals[i].tensor[6] = real4;
        epsilonList_.epsilonVals[i].tensor[7] = -imag4;
        epsilonList_.epsilonVals[i].tensor[8] = real5;
        epsilonList_.epsilonVals[i].tensor[9] = -imag5;
      }
    }
    inputFile2.close();
  }

  /*==============================================*/
  // Function returning the omega values
  /*==============================================*/
  double* FileLoader::getOmegaList(){
    return omegaList_;
  }
  /*==============================================*/
  // Function returning the epsilon values
  /*==============================================*/
  EPSILON FileLoader::getEpsilonList(){
    return epsilonList_;
  }
  /*==============================================*/
  // Function returning number of omega points
  /*==============================================*/
  int FileLoader::getNumOfOmega(){
    return numOfOmega_;
  }
  /*==============================================*/
  // Class destructor
  /*==============================================*/
  FileLoader::~FileLoader(){
    if(omegaList_ != nullptr){
      delete [] omegaList_;
      omegaList_ = nullptr;
    }
    if(epsilonList_.epsilonVals != nullptr){
      delete [] epsilonList_.epsilonVals;
      epsilonList_.epsilonVals = nullptr;
    }
  }
  /*==============================================*/
  // This function wraps the data for quad_gaussian_kronrod
  // @args:
  // kx: the kx value (normalized)
  // wrapper: wrapper for all the arguments wrapped in wrapper
  /*==============================================*/
  static void wrapperFunQuadgk(unsigned ndim,
    const double *kx,
    void* data,
    unsigned fdim,
    double *fval
    ){
    ArgWrapper wrapper = *(ArgWrapper*)data;
    fval[0] = kx[0] * poyntingFlux(
      wrapper.omega / MICRON,
      wrapper.thicknessList,
      kx[0],
      0,
      wrapper.EMatrices,
      wrapper.grandImaginaryMatrices,
      wrapper.eps_zz_Inv,
      wrapper.Gx_mat,
      wrapper.Gy_mat,
      wrapper.sourceList,
      wrapper.targetLayer,
      1,
      wrapper.polar
    );
  }
  /*==============================================*/
  // This function wraps the data for quad_legendre
  // @args:
  // kx: the kx value (normalized)
  // data: wrapper for all the arguments wrapped in wrapper
  /*==============================================*/
  static double wrapperFunQuadgl(const double kx, void* data){
    ArgWrapper wrapper = *(ArgWrapper*) data;
    return kx * poyntingFlux(
      wrapper.omega / MICRON,
      wrapper.thicknessList,
      kx,
      0,
      wrapper.EMatrices,
      wrapper.grandImaginaryMatrices,
      wrapper.eps_zz_Inv,
      wrapper.Gx_mat,
      wrapper.Gy_mat,
      wrapper.sourceList,
      wrapper.targetLayer,
      1,
      wrapper.polar
    );
  }
  /*======================================================*/
  // Implementaion of the parent simulation super class
  /*=======================================================*/
  Simulation::Simulation() : nG_(0), numOfOmega_(0), Phi_(nullptr), omegaList_(nullptr),
   kxStart_(0), kxEnd_(0), kyStart_(0), kyEnd_(0), numOfKx_(0), numOfKy_(0)
  {
    targetLayer_ = -1;
    dim_ = NO_;
    structure_ = Structure::instanceNew();
    fileLoader_ = FileLoader::instanceNew();

  }
  /*==============================================*/
  // Class destructor
  /*==============================================*/
  Simulation::~Simulation(){
    if(Phi_ != nullptr){
      delete[] Phi_;
      Phi_ = nullptr;
    }
  }


  /*==============================================*/
  // This function return the Phi value
  /*==============================================*/
  double* Simulation::getPhi(){

    if(Phi_ == nullptr){
      std::cerr << "Please do integration first!" << std::endl;
      throw UTILITY::MemoryException("Please do integration first!");
    }
    return Phi_;
  }

  /*==============================================*/
  // This function return the omega value
  /*==============================================*/
  double* Simulation::getOmega(){
    if(omegaList_ == nullptr){
      std::cerr << "omega does not exist!" << std::endl;
      throw UTILITY::MemoryException("omega does not exist!");
    }
    return omegaList_;
  }
  /*==============================================*/
  // This function return reconstructed dielectric at a given point
  // @args:
  // omegaIndex: the index of the omega
  // position: the spacial position in SI unit
  // epsilon: the ouput epsilon values
  /*==============================================*/
  void Simulation::getEpsilon(const int omegaIndex, const double position[3], double* &epsilon){
    double positions[3] = {position[0] * MICRON, position[1] * MICRON, position[2] * MICRON};
    if(omegaIndex < 0 || omegaIndex >= numOfOmega_){
      std::cerr << "index out of range!" << std::endl;
      throw UTILITY::RangeException("index out of range!");
    }
    if(omegaIndex != curOmegaIndex_){
      curOmegaIndex_ = omegaIndex;
      this->buildRCWAMatrices();
    }
    int layerIdx = 0;
    double offset = 0;
    for(int i = 1; i < structure_->getNumOfLayer(); i++){
      if(positions[2] > offset && positions[2] <= offset + thicknessListVec_(i)){
        layerIdx = i;
        break;
      }
      offset += thicknessListVec_(i);
    }
    if(layerIdx == 0 && positions[2] > offset) layerIdx = structure_->getNumOfLayer() - 1;

    RCWArMatrix Gx_r, Gx_l, Gy_r, Gy_l;
    meshGrid(Gx_mat_, Gx_mat_, Gx_r, Gx_l);
    meshGrid(Gy_mat_, Gy_mat_, Gy_r, Gy_l);

    RCWArMatrix GxMat = Gx_l - Gx_r;
    RCWArMatrix GyMat = Gy_l - Gy_r;
    int r1 = 0, r2 = nG_-1, r3 = nG_, r4 = 2*nG_-1;
    int pos = (nG_-1)/2;
    if(options_.truncation_ == CIRCULAR_ && dim_ == TWO_) pos = 0;
    arma::cx_rowvec phase = exp(-IMAG_I * (GxMat.row(pos) * positions[0] + GyMat.row(pos) * positions[1]));
    dcomplex eps_xx = accu(EMatrices_[layerIdx](span(r3, r4), span(r3, r4)).row(pos) % phase);
    dcomplex eps_xy = -accu(EMatrices_[layerIdx](span(r3, r4), span(r1, r2)).row(pos) % phase);
    dcomplex eps_yx = -accu(EMatrices_[layerIdx](span(r1, r2), span(r3, r4)).row(pos) % phase);
    dcomplex eps_yy = accu(EMatrices_[layerIdx](span(r1, r2), span(r1, r2)).row(pos) % phase);
    RCWAcMatrix eps_zz_mat =inv(eps_zz_Inv_Matrices_[layerIdx]);
    dcomplex eps_zz = accu(eps_zz_mat.row(pos) % phase);

    epsilon[0] = real(eps_xx);
    epsilon[1] = -imag(eps_xx);
    epsilon[2] = real(eps_xy);
    epsilon[3] = -imag(eps_xy);
    epsilon[4] = real(eps_yx);
    epsilon[5] = -imag(eps_yx);
    epsilon[6] = real(eps_yy);
    epsilon[7] = -imag(eps_yy);
    epsilon[8] = real(eps_zz);
    epsilon[9] = -imag(eps_zz);
  }
  /*==============================================*/
  // This function return reconstructed dielectric at a given layer
  // @args:
  // omegaIndex: the index of the omega
  // name: the name of the layer
  // Nu: number of points in x direction
  // Nv: number of points in y direction
  // fileName: optional, the ouput file
  /*==============================================*/
  void Simulation::outputLayerPatternRealization(
    const int omegaIndex,
    const std::string name,
    const int Nu,
    const int Nv,
    const std::string fileName
  ){
    if(Nu <= 0 || Nv <= 0){
      std::cerr << "Number of point needs to be positive!" << std::endl;
      throw UTILITY::RangeException("Number of point needs to be positive!");
    }
    double dx, dy;
    if(Nu == 1) dx = lattice_.bx[0];
    else dx = lattice_.bx[0] / (Nu - 1);

    if(Nv == 1) dy = hypot(lattice_.by[0], lattice_.by[1]);
    else dy = hypot(lattice_.by[0], lattice_.by[1]) / (Nv - 1);

    double position[3] = {0, 0, 0};
    double* epsilon = new double[10];
    for(const_LayerInstanceIter it = layerInstanceMap_.cbegin(); it != layerInstanceMap_.cend(); it++){
      std::string thisLayerName = it->first;
      if(name.compare(thisLayerName) != 0){
        position[2] += (it->second)->getThickness();
      }
      else{
        position[2] += (it->second)->getThickness() / 2;
        // if it is the top layer, be careful
        if(it == layerInstanceMap_.cend()) position[2] *= 2;
        break;
      }
    }

    std::ofstream outputFile;
    if(fileName != ""){
      outputFile.open(fileName);
    }

    for(int i = 0; i < Nu; i++){
      for(int j = 0; j < Nv; j++){
        position[0] = dx * i + dy * j * sin(lattice_.angle * datum::pi / 180);
        position[1] = dy * j * cos(lattice_.angle * datum::pi / 180);
        this->getEpsilon(omegaIndex, position, epsilon);
        if(fileName != ""){
          outputFile << position[0] << "\t" << position[1];
          for(int k = 0; k < 10; k++) outputFile << "\t" << epsilon[k];
          outputFile << std::endl;
        }
        else{
          std::cout << position[0] << "\t" << position[1] << "\t";
          for(int k = 0; k < 10; k++) std::cout << "\t" << epsilon[k];
          std::cout << std::endl;
        }
      }
    }

    if(fileName != "") outputFile.close();
    delete [] epsilon;
  }
  /*==============================================*/
  // This function return the number of omega
  /*==============================================*/
  int Simulation::getNumOfOmega(){
    return numOfOmega_;
  }
  /*==============================================*/
  // This function adds material to the system
  // @args:
  // name: the name of the material
  // infile: the input file containing the properties of the material
  /*==============================================*/
  void Simulation::addMaterial(const std::string name, const std::string infile){
    if(materialInstanceMap_.find(name) != materialInstanceMap_.cend()){
      std::cerr << name + ": Material already exist!" << std::endl;
      throw UTILITY::NameInUseException(name + ": Material already exist!");
      return;
    }

    fileLoader_->load(infile);
    Ptr<Material> material = Material::instanceNew(name,
     fileLoader_->getOmegaList(),
     fileLoader_->getEpsilonList(),
     fileLoader_->getNumOfOmega()
    );
    materialInstanceMap_.insert(MaterialMap::value_type(name, material));
    structure_->addMaterial(material);
  }

  /*==============================================*/
  // This function reset the dielectric of a material
  // @args:
  // name: the name of the material
  // epsilon: the new epsilon values
  // type: the type of the epsilon, one of ['scalar', 'diagonal', 'tensor']
  // Note: the material should have already existed in the system
  // If type == 'scalar', epsilon should have size [numOfOmega][2]
  // If type == 'diagonal', epsilon should have size [numOfOmega][6]
  // If type == 'tensor', epsilon should have size [numOfOmega][10]
  /*==============================================*/
  void Simulation::setMaterial(const std::string name, double** &epsilon, const std::string type){
    if(materialInstanceMap_.find(name) == materialInstanceMap_.cend()){
      std::cerr << name + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(name + ": Material does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(name)->second;
    EPSTYPE originalType = material->getType();
    int numOfOmega = material->getNumOfOmega();
    EPSILON newEpsilon;
    newEpsilon.epsilonVals = new EpsilonVal[numOfOmega];
    // if a scalar
    if(type == "scalar"){
      for(int i = 0; i < numOfOmega; i++){
        newEpsilon.epsilonVals[i].scalar[0] = epsilon[i][0];
        newEpsilon.epsilonVals[i].scalar[1] = epsilon[i][1];
      }
      newEpsilon.type_ = SCALAR_;
    }
    // if a diagonal
    else if(type == "diagonal"){
      for(int i = 0; i < numOfOmega; i++){
        for(int j = 0; j < 6; j++){
          newEpsilon.epsilonVals[i].scalar[j] = epsilon[i][j];
        }
      }
      newEpsilon.type_ = DIAGONAL_;
    }
    // if a tensor
    else if(type == "tensor"){
      for(int i = 0; i < numOfOmega; i++){
        for(int j = 0; j < 10; j++){
          newEpsilon.epsilonVals[i].scalar[j] = epsilon[i][j];
        }
      }
      newEpsilon.type_ = TENSOR_;
    }
    // error
    else{
      std::cerr << "Please choose 'type' from 'scalar', 'diagonal' or 'tensor'!" << std::endl;
      throw UTILITY::AttributeNotSupportedException("Please choose 'type' from 'scalar', 'diagonal' or 'tensor'!");
    }

    material->setEpsilon(newEpsilon, numOfOmega);
    delete [] newEpsilon.epsilonVals;
    newEpsilon.epsilonVals = nullptr;

    for(const_LayerInstanceIter it = layerInstanceMap_.cbegin(); it != layerInstanceMap_.cend(); it++){
      Ptr<Layer> layer = it->second;
      if(layer->hasMaterial(material)){

        if(originalType != TENSOR_ && type == "tensor"){
          layer->containTensor(true);
        }
        if(originalType == TENSOR_ && type != "tensor"){
          layer->containTensor(false);
        }

      }
    }

  }
  /*==============================================*/
  // This function adds a new layer to the system
  // @args:
  // name: the name of the layer
  // thick: the thickness of the layer
  // materialName: the name of the material
  /*==============================================*/
  void Simulation::addLayer(const std::string name, const double thick, const std::string materialName){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(name) != layerInstanceMap_.cend()){
      std::cerr << name + ": Layer already exists!" << std::endl;
      throw UTILITY::NameInUseException(name + ": Layer already exists!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = Layer::instanceNew(name, material, thick);
    structure_->addLayer(layer);
    layerInstanceMap_.insert(LayerInstanceMap::value_type(name, layer));
  }
  /*==============================================*/
  // This function change the background material of a layer and its thickness
  // @args:
  // name: the name of the layer
  // thick: the new thickness
  // materialName: the new background
  /*==============================================*/
  void Simulation::setLayer(const std::string name, const double thick, const std::string materialName){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(name) == layerInstanceMap_.cend()){
      std::cerr << name + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(name + ": Layer does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(name)->second;
    if(material->getType() == TENSOR_ && !layer->hasMaterial(material)){
      layer->containTensor(true);
    }
    if((layer->getBackGround())->getType() == TENSOR_ && material->getType() != TENSOR_){
      layer->containTensor(false);
    }
    layer->setBackGround(material);
    layer->setThickness(thick);
  }
  /*==============================================*/
  // This function change the thickness
  // @args:
  // name: the name of the layer
  // thick: the new thickness
  /*==============================================*/
  void Simulation::setLayerThickness(const std::string name, const double thick){
    if(layerInstanceMap_.find(name) == layerInstanceMap_.cend()){
      std::cerr << name + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(name + ": Layer does not exist!");
      return;
    }
    Ptr<Layer> layer = layerInstanceMap_.find(name)->second;
    layer->setThickness(thick);
  }
  /*==============================================*/
  // This function make a copy of an existing layer
  // @args:
  // name: the name of the copied layer
  // originalName: the name of the original layer
  /*==============================================*/
  void Simulation::addLayerCopy(const std::string name, const std::string originalName){
    if(layerInstanceMap_.find(originalName) == layerInstanceMap_.cend()){
      std::cerr << originalName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(originalName + ": Layer does not exist!");
      return;
    }
    if(layerInstanceMap_.find(name) != layerInstanceMap_.cend()){
      std::cerr << name + ": cannot add a layer that already exists!" << std::endl;
      throw UTILITY::NameInUseException(name + ": cannot add a layer that already exists!");
      return;
    }
    Ptr<Layer> originalLayer = layerInstanceMap_.find(originalName)->second;
    Ptr<Layer> newLayer = originalLayer->layerCopy(name);
    layerInstanceMap_.insert(LayerInstanceMap::value_type(name, newLayer));
    structure_->addLayer(newLayer);
  }
  /*==============================================*/
  // This function deletes an existing layer
  // @args:
  // name: the name of the layer
  /*==============================================*/
  void Simulation::deleteLayer(const std::string name){
    if(layerInstanceMap_.find(name) == layerInstanceMap_.cend()){
      std::cerr << name + ": Layer does not exist!" << std::endl;
      throw UTILITY::NameInUseException(name + ": Layer does not exist!");
      return;
    }
    layerInstanceMap_.erase(name);
    structure_->deleteLayerByName(name);
  }
  /*==============================================*/
  // This function sets a layer as the source
  // @args:
  // name: the name of the source layer
  /*==============================================*/
  void Simulation::setSourceLayer(const std::string name){
    if(layerInstanceMap_.find(name) == layerInstanceMap_.cend()){
      std::cerr << name + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(name + ": Layer does not exist!");
      return;
    }
    Ptr<Layer> layer = layerInstanceMap_.find(name)->second;
    layer->setIsSource();
  }
  /*==============================================*/
  // This function sets the probe layer
  // @args:
  // name: the name of the probe layer
  /*==============================================*/
  void Simulation::setProbeLayer(const std::string name){
    if(layerInstanceMap_.find(name) == layerInstanceMap_.cend()){
      std::cerr << name + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(name + ": Layer does not exist!");
      return;
    }
    Ptr<Layer> layer = layerInstanceMap_.find(name)->second;
    this->setTargetLayerByLayer(layer);
  }
  /*==============================================*/
  // This function sets number of G
  // @args:
  // nG: number of G
  /*==============================================*/
  void Simulation::setNumOfG(const int nG){
    nG_ = nG;
  }

  /*==============================================*/
  // This function sets the target layer by layer
  // @args:
  // index: target layer
  /*==============================================*/
  void Simulation::setTargetLayerByLayer(const Ptr<Layer>& layer){
    for(int i = 0; i < structure_->getNumOfLayer(); i++){
      if(structure_->getLayerByIndex(i) == layer){
        targetLayer_ = i;
        return;
      }
    }
  }

  /*==============================================*/
  // This function cleans up the simulation
  /*==============================================*/
  void Simulation::resetSimulation(){
    EMatrices_.clear();
    grandImaginaryMatrices_.clear();
    eps_zz_Inv_Matrices_.clear();
    sourceList_.clear();
    thicknessListVec_.clear();
    curOmegaIndex_ = -1;
  }

  /*==============================================*/
  // This function gets the structure
  /*==============================================*/
  Ptr<Structure> Simulation::getStructure(){
    return structure_;
  }

  /*==============================================*/
  // This function gets the Phi at given kx and ky
  // @args:
  // omegaIndex: the index of omega
  // kx: the kx value, normalized
  // ky: the ky value, normalized
  // @note
  // used by grating and patterning
  // N: the number of total G
  /*==============================================*/
  double Simulation::getPhiAtKxKy(const int omegaIdx, const double kx, const double ky){
    if(omegaIdx >= numOfOmega_){
      std::cerr << std::to_string(omegaIdx) + ": out of range!" << std::endl;
      throw UTILITY::RangeException(std::to_string(omegaIdx) + ": out of range!");
    }
    if(curOmegaIndex_ != omegaIdx){
      curOmegaIndex_ = omegaIdx;
      this->buildRCWAMatrices();
    }
    return omegaList_[omegaIdx] / datum::c_0 / POW3(datum::pi) / 2.0 *
      poyntingFlux(omegaList_[omegaIdx] / datum::c_0 / MICRON,
        thicknessListVec_,
        kx,
        ky,
        EMatrices_,
        grandImaginaryMatrices_,
        eps_zz_Inv_Matrices_,
        Gx_mat_,
        Gy_mat_,
        sourceList_,
        targetLayer_,
        nG_,
        options_.polarization
      );
  }
  /*==============================================*/
  // function return the number of G values
  /*==============================================*/
  int Simulation::getNumOfG(){
    return nG_;
  }

  /*==============================================*/
  // This function intializes the simulation
  // Will initialize numOfOmega_, omegaList_, thicknessListVec_, sourceList_
  /*==============================================*/
  void Simulation::initSimulation(){
    // reset simulation first
    this->resetSimulation();
    // essential, get the shared Gx_mat_ and Gy_mat_
    Lattice rescaledLattice;
    rescaledLattice.bx[0] = reciprocalLattice_.bx[0] / MICRON;
    rescaledLattice.bx[1] = reciprocalLattice_.bx[1] / MICRON;
    rescaledLattice.by[0] = reciprocalLattice_.by[0] / MICRON;
    rescaledLattice.by[1] = reciprocalLattice_.by[1] / MICRON;
    rescaledLattice.angle = reciprocalLattice_.angle;
    if(dim_ == ONE_){
      rescaledLattice.area = reciprocalLattice_.area / MICRON;
    }
    else{
      rescaledLattice.area = reciprocalLattice_.area / POW2(MICRON);
    }
    GSEL::getGMatrices(nG_, rescaledLattice, Gx_mat_, Gy_mat_, dim_, options_.truncation_);
    // get constants
    Ptr<Layer> firstLayer = structure_->getLayerByIndex(0);
    Ptr<Material> backGround = firstLayer->getBackGround();
    numOfOmega_ = backGround->getNumOfOmega();
    omegaList_ = backGround->getOmegaList();
    int numOfLayer = structure_->getNumOfLayer();

    EMatrices_.resize(numOfLayer);
    grandImaginaryMatrices_.resize(numOfLayer);
    eps_zz_Inv_Matrices_.resize(numOfLayer);

    thicknessListVec_ = zeros<RCWArVector>(numOfLayer);
    sourceList_.resize(numOfLayer);
    for(int i = 0; i < numOfLayer; i++){
      thicknessListVec_(i) = (structure_->getLayerByIndex(i))->getThickness() * MICRON;
      sourceList_[i] = (structure_->getLayerByIndex(i))->checkIsSource();
      if(sourceList_[i] && i > targetLayer_){
        std::cerr << "Probe layer cannot be lower than source layer!" << std::endl;
        throw UTILITY::RangeException("Probe layer cannot be lower than source layer!");
      }
    }
    // set the first and last layer to have 0 thickness
    thicknessListVec_(0) = 0;
    thicknessListVec_(numOfLayer - 1) = 0;

    if(dim_ != NO_ && reciprocalLattice_.bx[0] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }
    if(dim_ == TWO_ && reciprocalLattice_.by[1] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }

    // initializing for the output
    Phi_ = new double[numOfOmega_];
    for(int i = 0; i < numOfOmega_; i++){
      Phi_[i] = 0;
    }
    // initialize layers
    for(int i = 0; i < numOfLayer; i++){
      Ptr<Layer> layer = structure_->getLayerByIndex(i);
      layer->getGeometryContainmentRelation();
    }
  }
  /*==============================================*/
  // This function builds up the matrices
  /*==============================================*/
  void Simulation::buildRCWAMatrices(){
    RCWAcMatrices eps_xx_Matrices, eps_xy_Matrices, eps_yx_Matrices, eps_yy_Matrices;
    RCWAcMatrices im_eps_xx_Matrices, im_eps_xy_Matrices, im_eps_yx_Matrices, im_eps_yy_Matrices, im_eps_zz_Matrices;

    RCWAcMatrix onePadding1N = eye<RCWAcMatrix>(nG_, nG_);
    int numOfLayer = structure_->getNumOfLayer();
    double area;
    if(dim_ == ONE_){
      area = lattice_.area * MICRON;
    }
    else {
      area = lattice_.area * POW2(MICRON);
    }
    for(int i = 0; i < numOfLayer; i++){
      Ptr<Layer> layer = structure_->getLayerByIndex(i);
      Ptr<Material> backGround = layer->getBackGround();

      RCWAcMatrix eps_xx(nG_, nG_, fill::zeros), eps_xy(nG_, nG_, fill::zeros), eps_yx(nG_, nG_, fill::zeros), eps_yy(nG_, nG_, fill::zeros), eps_zz(nG_, nG_, fill::zeros), eps_zz_Inv(nG_, nG_, fill::zeros);
      RCWAcMatrix im_eps_xx(nG_, nG_, fill::zeros), im_eps_xy(nG_, nG_, fill::zeros), im_eps_yx(nG_, nG_, fill::zeros), im_eps_yy(nG_, nG_, fill::zeros), im_eps_zz(nG_, nG_, fill::zeros);

      EpsilonVal epsBG = backGround->getEpsilonAtIndex(curOmegaIndex_);
      EpsilonVal epsBGTensor = FMM::toTensor(epsBG, backGround->getType());
      const_MaterialIter m_it = layer->getMaterialsBegin();
      int count = 0;
      for(const_PatternIter it = layer->getPatternsBegin(); it != layer->getPatternsEnd(); it++){
        Pattern pattern = *it;
        Ptr<Material> material = *(m_it + count);
        EpsilonVal epsilon = material->getEpsilonAtIndex(curOmegaIndex_);
        count++;
        EpsilonVal epsParentTensor;
        if(pattern.parent == -1){
          epsParentTensor = epsBGTensor;
        }
        else{
          Ptr<Material> materialParent = *(m_it + pattern.parent);
          EpsilonVal epsParent = materialParent->getEpsilonAtIndex(curOmegaIndex_);
          epsParentTensor = FMM::toTensor(epsParent, materialParent->getType());
        }
        switch(pattern.type_){
          /*************************************/
          // if the pattern is a grating (1D)
          /************************************/
          case GRATING_:{
            double center = pattern.arg1_.first * MICRON;
            double width = pattern.arg1_.second * MICRON;
            FMM::transformGrating(
              eps_xx,
              eps_xy,
              eps_yx,
              eps_yy,
              eps_zz,
              im_eps_xx,
              im_eps_xy,
              im_eps_yx,
              im_eps_yy,
              im_eps_zz,
              epsParentTensor,
              epsilon,
              material->getType(),
              Gx_mat_,
              center,
              width,
              area,
              layer->hasTensor()
            );
            break;
          }

          /*************************************/
          // if the pattern is a rectangle (2D)
          /************************************/
          case RECTANGLE_:{
            double centers[2] = {pattern.arg1_.first * MICRON, pattern.arg1_.second * MICRON};
            double widths[2] = {pattern.arg2_.first * MICRON, pattern.arg2_.second * MICRON};
            double angle = datum::pi / 180 * pattern.angle_;
            FMM::transformRectangle(
              eps_xx,
              eps_xy,
              eps_yx,
              eps_yy,
              eps_zz,
              im_eps_xx,
              im_eps_xy,
              im_eps_yx,
              im_eps_yy,
              im_eps_zz,
              epsParentTensor,
              epsilon,
              material->getType(),
              Gx_mat_,
              Gy_mat_,
              centers,
              angle,
              widths,
              area,
              layer->hasTensor()
            );
            break;
          }
          /*************************************/
          // if the pattern is a circle (2D)
          /************************************/
          case CIRCLE_:{
            double centers[2] = {pattern.arg1_.first * MICRON, pattern.arg2_.first * MICRON};
            double radius = pattern.arg1_.second * MICRON;
            FMM::transformCircle(
              eps_xx,
              eps_xy,
              eps_yx,
              eps_yy,
              eps_zz,
              im_eps_xx,
              im_eps_xy,
              im_eps_yx,
              im_eps_yy,
              im_eps_zz,
              epsParentTensor,
              epsilon,
              material->getType(),
              Gx_mat_,
              Gy_mat_,
              centers,
              radius,
              area,
              layer->hasTensor()
            );
            break;
          }
          /*************************************/
          // if the pattern is an ellipse (2D)
          /************************************/
          case ELLIPSE_:{
            double centers[2] = {pattern.arg1_.first * MICRON, pattern.arg1_.second * MICRON};
            double halfwidths[2] = {pattern.arg2_.first * MICRON, pattern.arg2_.second * MICRON};
            double angle = datum::pi / 180 * pattern.angle_;
            FMM::transformEllipse(
              eps_xx,
              eps_xy,
              eps_yx,
              eps_yy,
              eps_zz,
              im_eps_xx,
              im_eps_xy,
              im_eps_yx,
              im_eps_yy,
              im_eps_zz,
              epsParentTensor,
              epsilon,
              material->getType(),
              Gx_mat_,
              Gy_mat_,
              centers,
              angle,
              halfwidths,
              area,
              layer->hasTensor()
            );
            break;
          }
          /*************************************/
          // if the pattern is a polygon (2D)
          /************************************/
          case POLYGON_:{
            double centers[2] = {pattern.arg1_.first * MICRON, pattern.arg1_.second * MICRON};
            EdgeList edgeList;
            for(size_t i = 0; i < pattern.edgeList_.size(); i++){
              edgeList.push_back(std::make_pair(pattern.edgeList_[i].first * MICRON, pattern.edgeList_[i].second * MICRON));
            }
            double angle = datum::pi / 180 * pattern.angle_;
            FMM::transformPolygon(
             eps_xx,
             eps_xy,
             eps_yx,
             eps_yy,
             eps_zz,
             im_eps_xx,
             im_eps_xy,
             im_eps_yx,
             im_eps_yy,
             im_eps_zz,
             epsParentTensor,
             epsilon,
             material->getType(),
             Gx_mat_,
             Gy_mat_,
             centers,
             angle,
             edgeList,
             area,
             layer->hasTensor()
           );
            break;
          }
          default: break;
        }
      }
      /*************************************/
      // collection information from the background
      /************************************/
      eps_xx += dcomplex(epsBGTensor.tensor[0], epsBGTensor.tensor[1]) * onePadding1N;
      im_eps_xx += epsBGTensor.tensor[1] * onePadding1N;

      eps_yy += dcomplex(epsBGTensor.tensor[6], epsBGTensor.tensor[7]) * onePadding1N;
      im_eps_yy += epsBGTensor.tensor[7] * onePadding1N;

      eps_zz += dcomplex(epsBGTensor.tensor[8], epsBGTensor.tensor[9]) * onePadding1N;
      eps_zz_Inv = eps_zz.i();

      im_eps_zz += epsBGTensor.tensor[9] * onePadding1N;

      if(layer->hasTensor()){
        eps_xy += dcomplex(epsBGTensor.tensor[2], epsBGTensor.tensor[3]) * onePadding1N;
        eps_yx += dcomplex(epsBGTensor.tensor[4], epsBGTensor.tensor[5]) * onePadding1N;
        //im_eps_xy += epsBGTensor.tensor[3] * onePadding1N;
        //im_eps_yx += epsBGTensor.tensor[5] * onePadding1N;
        im_eps_xy += ( dcomplex(epsBGTensor.tensor[2], epsBGTensor.tensor[3]) - dcomplex(epsBGTensor.tensor[4], -epsBGTensor.tensor[5]) )
            / 2.0 / IMAG_I * onePadding1N;
        im_eps_yx += ( dcomplex(epsBGTensor.tensor[4], epsBGTensor.tensor[5]) - dcomplex(epsBGTensor.tensor[2], -epsBGTensor.tensor[3]) )
            / 2.0 / IMAG_I * onePadding1N;
      }

      eps_xx_Matrices.push_back(eps_xx);
      eps_xy_Matrices.push_back(eps_xy);
      eps_yx_Matrices.push_back(eps_yx);
      eps_yy_Matrices.push_back(eps_yy);
      eps_zz_Inv_Matrices_[i] = eps_zz_Inv;
      im_eps_xx_Matrices.push_back(im_eps_xx);
      im_eps_xy_Matrices.push_back(im_eps_xy);
      im_eps_yx_Matrices.push_back(im_eps_yx);
      im_eps_yy_Matrices.push_back(im_eps_yy);
      im_eps_zz_Matrices.push_back(im_eps_zz);
    }

    getEMatrices(
      EMatrices_,
      eps_xx_Matrices,
      eps_xy_Matrices,
      eps_yx_Matrices,
      eps_yy_Matrices,
      numOfLayer,
      nG_
    );

    getGrandImaginaryMatrices(
      grandImaginaryMatrices_,
      im_eps_xx_Matrices,
      im_eps_xy_Matrices,
      im_eps_yx_Matrices,
      im_eps_yy_Matrices,
      im_eps_zz_Matrices,
      numOfLayer,
      nG_
    );
  }
  /*==============================================*/
  // This function prints out the information of the system
  /*==============================================*/
  void Simulation::outputSysInfo(){
    std::cout << "==================================================" << std::endl;
    std::cout << "The system has in total " << structure_->getNumOfLayer() << " layers." << std::endl;
    if(dim_ == ONE_){
      std::cout << "Periodicity in x direction is " << lattice_.bx[0] << std::endl;
    }
    else if(dim_ == TWO_){
      std::cout << "Lattice coordinates are (" << lattice_.bx[0] << "," << lattice_.bx[1] << "), ("
       << lattice_.by[0] << ", " << lattice_.by[1] << ")" << std::endl;
    }
    std::cout << "==================================================" << std::endl;
    std::cout << "Printing from bottom to up." << std::endl;
    std::cout << "==================================================" << std::endl;
    for(const_LayerIter it = structure_->getLayersBegin(); it != structure_->getLayersEnd(); it++){
      Ptr<Layer> layer = it->second;
      layer->getGeometryContainmentRelation();
      std::cout << "Layer index " << it->first << ": " << layer->getName() << std::endl;
      std::cout << "Thickness: " << layer->getThickness() << std::endl;

      std::cout << "contains off diagonal epsilon: ";
      if(layer->hasTensor()) std::cout << "YES" << std::endl;
      else std::cout << "NO" << std::endl;

      std::cout << "Is source: ";
      if(layer->checkIsSource()) std::cout << "YES" << std::endl;
      else std::cout << "NO" << std::endl;

      std::cout << "Its background is: " << layer->getBackGround()->getName() << std::endl;
      if(layer->getNumOfMaterial() != 0){
        std::cout << "It has other components:" << std::endl << std::endl;
        int count = 0;
        const_MaterialIter m_it = layer->getMaterialsBegin();
        for(const_PatternIter it = layer->getPatternsBegin(); it != layer->getPatternsEnd(); it++){
          std::cout << "Material for pattern " << count + 1 << ": " << (*(m_it + count))->getName() << std::endl;
          std::cout << "Pattern " << count + 1 << " is: ";
          switch(it->type_){
            case GRATING_:{
              std::cout << "grating, ";
              std::cout << "(c, w) = (" << (*it).arg1_.first << ", " << (*it).arg1_.second << ")\n";
              break;
            }
            case RECTANGLE_:{
              std::cout << "rectangle, ";
              std::cout << "(c_x, w_x) = (" << (*it).arg1_.first << ", " << (*it).arg2_.first <<"), ";
              std::cout << "(c_y, w_y) = (" << (*it).arg1_.second << ", " << (*it).arg2_.second <<")\n";
              std::cout << "angle = " << (*it).angle_ << std::endl;
              break;
            }
            case CIRCLE_:{
              std::cout << "circle, ";
              std::cout << "(c_x, c_y) = (" << (*it).arg1_.first << ", " << (*it).arg2_.first << "), ";
              std::cout << "r = " << (*it).arg1_.second << std::endl;
              break;
            }
            case ELLIPSE_:{
              std::cout << "ellipse, ";
              std::cout << "(c_x, a) = (" << (*it).arg1_.first << ", " << (*it).arg2_.first <<"), ";
              std::cout << "(c_y, b) = (" << (*it).arg1_.second << ", " << (*it).arg2_.second <<")\n";
              std::cout << "angle = " << (*it).angle_ << std::endl;
              break;
            }
            case POLYGON_:{
              std::cout << "polygon, ";
              std::cout << "(c_x, c_y) = (" << (*it).arg1_.first << ", " << (*it).arg1_.second << ")\n";
              std::cout << "angle = " << (*it).angle_ << std::endl;
              std::cout << "==> print vertices in counterclockwise order:" << std::endl;
              for(size_t i = 0; i < (*it).edgeList_.size(); i++){
                std::cout << "==> (x" << i + 1 <<", y" << i + 1 << ") = (" << (*it).edgeList_[i].first + (*it).arg1_.first << ", " << (*it).edgeList_[i].second + (*it).arg1_.second << "),\n";
              }
              break;
            }
            default: break;
          }
          if(it->parent != -1){
            std::cout << "**** contained in pattern " << it->parent + 1 << std::endl;
          }
          count++;
        }
      }
      std::cout << "==================================================" << std::endl;
    }
  }
  /*==============================================*/
  // function print intermediate results
  /*==============================================*/
  void Simulation::optPrintIntermediate(){
    options_.PrintIntermediate = true;
  }
  /*==============================================*/
  // function sets that only TE mode is computed
  /*==============================================*/
  void Simulation::optOnlyComputeTE(){
    options_.polarization = TE_;
  }
  /*==============================================*/
  // function sets that only TM mode is computed
  /*==============================================*/
  void Simulation::optOnlyComputeTM(){
    options_.polarization = TM_;
  }
  /*==============================================*/
  // function sets the truncation of lattice
  // @args:
  //  truncation: string, one of "Circular" and "Parallelogramic"
  /*==============================================*/
  void Simulation::optSetLatticeTruncation(const std::string &truncation){
    if(truncation.compare("Circular") == 0){
      options_.truncation_ = CIRCULAR_;
    }
    else if(truncation.compare("Parallelogramic") == 0){
      options_.truncation_ = PARALLELOGRAMIC_;
    }
    else{
      std::cerr << "truncation should be one of Circular or Parallelogramic!" << std::endl;
      UTILITY::ValueException("truncation should be one of Circular or Parallelogramic!");
    }
  }

  /*==============================================*/
  // function print intermediate results
  /*==============================================*/
  void Simulation::setThread(const int thread){
    if(thread <= 0){
      std::cerr << "Number of thread should >= 1!" << std::endl;
      throw UTILITY::RangeException("Number of thread should >= 1!");
    }
    #if defined(_OPENMP)
      numOfThread_ = std::min(thread, omp_get_max_threads());
    #endif
  }
  /*==============================================*/
  // Function setting the integral over kx
  // @args:
  // points: number of points of sampling kx
  // end: the upperbound of the integral
  /*==============================================*/
  void Simulation::setKxIntegral(const int points, const double end){
    if(points < 2){
      std::cerr << "Needs no less than 2 points!" << std::endl;
      throw UTILITY::ValueException("Needs no less than 2 points!");
    }
    numOfKx_ = points;
    if(dim_ != NO_ && reciprocalLattice_.bx[0] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }
    if(dim_ == NO_ && end == 0.0){
      std::cerr << "integral upper bound cannot be zero!" << std::endl;
      throw UTILITY::ValueException("integral upper bound cannot be zero!");
    }
    if(end != 0){
      kxEnd_ = end;
      options_.kxIntegralPreset = true;
    }
    else{
      kxEnd_ = hypot(reciprocalLattice_.bx[0], reciprocalLattice_.bx[1]) / 2;
      options_.kxIntegralPreset = false;
    }
    kxStart_ = -kxEnd_;
  }

  /*==============================================*/
  // This function set the integral of kx when the system is symmetric in x direction
  // @args:
  // points: number of kx points
  // end: the upperbound of the integral
  /*==============================================*/
  void Simulation::setKxIntegralSym(const int points, const double end){
    if(points < 2){
      std::cerr << "Needs no less than 2 points!" << std::endl;
      throw UTILITY::ValueException("Needs no less than 2 points!");
    }
    numOfKx_ = points;
    if(dim_ != NO_ && reciprocalLattice_.bx[0] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }
    if(dim_ == NO_ && end == 0.0){
      std::cerr << "integral upper bound cannot be zero!" << std::endl;
      throw UTILITY::ValueException("integral upper bound cannot be zero!");
    }
    if(end != 0){
      kxEnd_ = end;
      options_.kxIntegralPreset = true;
    }
    else{
      kxEnd_ = hypot(reciprocalLattice_.bx[0], reciprocalLattice_.bx[1]) / 2;
      options_.kxIntegralPreset = false;
    }
    kxStart_ = 0;
    prefactor_ *= 2;
  }

  /*==============================================*/
  // This function set the integral of ky
  // @args:
  // points: number of ky points
  // end: the upperbound of the integral
  /*==============================================*/
  void Simulation::setKyIntegral(const int points, const double end){
    if(points < 2){
      std::cerr << "Needs no less than 2 points!" << std::endl;
      throw UTILITY::ValueException("Needs no less than 2 points!");
    }
    numOfKy_ = points;
    if(dim_ == TWO_ && reciprocalLattice_.by[1] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }
    if((dim_ == NO_ || dim_ == ONE_) && end == 0.0){
      std::cerr << "integral upper bound cannot be zero!" << std::endl;
      throw UTILITY::ValueException("integral upper bound cannot be zero!");
    }
    if(end != 0){
      kyEnd_ = end;
      options_.kyIntegralPreset = true;
    }
    else{
      kyEnd_ = hypot(reciprocalLattice_.by[0], reciprocalLattice_.by[1]) / 2;
      options_.kyIntegralPreset = false;
    }
    kyStart_ = -kyEnd_;
  }
  /*==============================================*/
  // This function set the integral of ky assume y symmetry
  // @args:
  // points: number of ky points
  // end: the upperbound of the integral
  /*==============================================*/
  void Simulation::setKyIntegralSym(const int points, const double end){
    if(points < 2){
      std::cerr << "Needs no less than 2 points!" << std::endl;
      throw UTILITY::ValueException("Needs no less than 2 points!");
    }
    numOfKy_ = points;
    if(dim_ == TWO_ && reciprocalLattice_.by[1] == 0.0){
      std::cerr << "Lattice not set!" << std::endl;
      throw UTILITY::ValueException("Lattice not set!");
    }
    if((dim_ == NO_ || dim_ == ONE_) && end == 0.0){
      std::cerr << "integral upper bound cannot be zero!" << std::endl;
      throw UTILITY::ValueException("integral upper bound cannot be zero!");
    }
    if(end != 0){
      kyEnd_ = end;
      options_.kyIntegralPreset = true;
    }
    else{
      kyEnd_ = hypot(reciprocalLattice_.by[0], reciprocalLattice_.by[1]) / 2;
      options_.kyIntegralPreset = false;
    }
    kyStart_ = 0;
    prefactor_ *= 2;
  }
  /*==============================================*/
  // This function computes the flux for internal usage
  // @args:
  // start: the starting index
  // end: the end index
  /*==============================================*/
  void Simulation::integrateKxKyInternal(const int start, const int end, const bool parallel){

    double* scalex = new double[numOfOmega_];
    double* scaley = new double[numOfOmega_];
    // here dkx is not normalized
    double dkx = (kxEnd_ - kxStart_) / (numOfKx_ - 1);
    // here kyEnd_ is normalized for 1D case
    double dky = (kyEnd_ - kyStart_) / (numOfKy_ - 1);

    for(int i = 0; i < numOfOmega_; i++){
      switch (dim_) {
        case NO_:{
          scalex[i] = 1;
          scaley[i] = 1;
          break;
        }
        case ONE_:{
          if(options_.kxIntegralPreset) scalex[i] = 1;
          else scalex[i] = omegaList_[i] / datum::c_0;
          scaley[i] = 1;
          break;
        }
        case TWO_:{
          if(options_.kxIntegralPreset) scalex[i] = 1;
          else scalex[i] = omegaList_[i] / datum::c_0;
          if(options_.kyIntegralPreset) scaley[i] = 1;
          else scaley[i] = omegaList_[i] / datum::c_0;
          break;
        }
        default: break;
      }
    }

    double* resultArray = new double[numOfKx_ * numOfKy_ * numOfOmega_];
    for(int i = 0; i < numOfKx_ * numOfKy_ * numOfOmega_; i++){
      resultArray[i] = 0;
    }

    //  this part is for the vanilla/openmp version of mesh
    if(parallel){
      double** kxList = new double*[numOfKx_];
      double** kyList = new double*[numOfKx_];
      for(int i = 0; i < numOfKx_; i++){
        kxList[i] = new double[numOfKy_];
        kyList[i] = new double[numOfKy_];
      }
      for(int omegaIdx = 0; omegaIdx < numOfOmega_; omegaIdx++){
        if(curOmegaIndex_ != omegaIdx){
          curOmegaIndex_ = omegaIdx;
          this->buildRCWAMatrices();
        }
        #if defined(_OPENMP)
          #pragma omp parallel for num_threads(numOfThread_), collapse(2)
        #endif
        for(int i = 0; i < numOfKx_; i++){
          for(int j = 0; j < numOfKy_; j++){
            double kx = kxStart_ + dkx * i;
            double ky = kyStart_ + dky * j;
            kxList[i][j] = (kx * cos((reciprocalLattice_.angle - 90) * datum::pi/180)) / scalex[omegaIdx];
            kyList[i][j] = (ky - kx * sin((reciprocalLattice_.angle - 90) * datum::pi/180)) / scaley[omegaIdx];
          }
        }
        #if defined(_OPENMP)
          #pragma omp parallel for num_threads(numOfThread_)
        #endif
        for(int i = 0; i < numOfKx_ * numOfKy_; i++){
          int kxIdx = i / numOfKy_;
          int kyIdx = i % numOfKy_;
          resultArray[omegaIdx * numOfKx_ * numOfKy_ + i] = this->getPhiAtKxKy(omegaIdx, kxList[kxIdx][kyIdx], kyList[kxIdx][kyIdx]);
          if(options_.PrintIntermediate){
            std::stringstream msg;
            msg << omegaList_[omegaIdx] << "\t" << kxList[kxIdx][kyIdx] << "\t" << kyList[kxIdx][kyIdx] << "\t" << resultArray[omegaIdx * numOfKx_ * numOfKy_ + i] << std::endl;
            std::cout << msg.str();
          }
        }
      }
      for(int i = 0; i < numOfKx_; i++){
        delete [] kxList[i];
        delete [] kyList[i];
      }
      delete[] kxList;
      kxList = nullptr;
      delete[] kyList;
      kyList = nullptr;
    }
    // this part is for the MPI version of mesh
    else{
      for(int i = start; i < end; i++){
          int omegaIdx = i / (numOfKx_ * numOfKy_);
          if(curOmegaIndex_ != omegaIdx){
            curOmegaIndex_ = omegaIdx;
            this->buildRCWAMatrices();
          }
          int residue = i % (numOfKx_ * numOfKy_);
          int kxIdx = residue / numOfKy_;
          int kyIdx = residue % numOfKy_;

          double kx = kxStart_ + dkx * kxIdx;
          double ky = kyStart_ + dky * kyIdx;

          ky = (ky - kx * sin((reciprocalLattice_.angle - 90) * datum::pi/180)) / scaley[omegaIdx];
          kx = (kx * cos((reciprocalLattice_.angle - 90) * datum::pi/180)) / scalex[omegaIdx];
          resultArray[i] = this->getPhiAtKxKy(omegaIdx, kx, ky);
          if(options_.PrintIntermediate){
            std::stringstream msg;
            msg << omegaList_[omegaIdx] << "\t" << kx << "\t" << ky << "\t" << resultArray[i] << std::endl;
            std::cout << msg.str();
          }
      }
    }

    for(int i = start; i < end; i++){
      int omegaIdx = i / (numOfKx_ * numOfKy_);
      Phi_[omegaIdx] += prefactor_ * resultArray[i] * dkx / scalex[omegaIdx] * dky / scaley[omegaIdx]
        * POW2(omegaList_[omegaIdx] / datum::c_0) * std::abs(sin(reciprocalLattice_.angle * datum::pi/180));
    }

    delete[] scalex;
    scalex = nullptr;
    delete[] scaley;
    scaley = nullptr;
    delete[] resultArray;
    resultArray = nullptr;
  }

  /*==============================================*/
  // This function computes the flux for general usage
  /*==============================================*/
  void Simulation::integrateKxKy(){
    this->integrateKxKyInternal(0, numOfOmega_ * numOfKx_ * numOfKy_, true);
  }
  /*==============================================*/
  // This function computes the flux for MPI only
  /*==============================================*/
  void Simulation::integrateKxKyMPI(const int rank, const int size){
    int totalNum = numOfOmega_ * numOfKx_ * numOfKy_;
    int chunksize = totalNum / size;
    int left = totalNum % size;
    int start, end;
    if(rank >= left){
      start = left * (chunksize + 1) + (rank - left) * chunksize;
      end = start + chunksize;
    }
    else{
      start = rank * (chunksize + 1);
      end = start + chunksize + 1;
    }
    if(end > totalNum) end = totalNum;
    this->integrateKxKyInternal(start, end, false);

  }
  /*==============================================*/
  // Implementaion of the class on planar simulation
  /*==============================================*/
  SimulationPlanar::SimulationPlanar() : Simulation(){
    dim_ = NO_;
    degree_ = DEGREE;
    prefactor_ = 1;
  }
  /*==============================================*/
  // This is a thin wrapper for the usage of smart pointer
  /*==============================================*/
  Ptr<SimulationPlanar> SimulationPlanar::instanceNew(){
    return new SimulationPlanar();
  };
  /*==============================================*/
  // Function setting the integral over kx
  // @args:
  // end: the end of the integration
  /*==============================================*/
  void SimulationPlanar::setKParallelIntegral(const double end){
    kxStart_ = 0;
    numOfKx_ = 0;
    kxEnd_ = end;
    options_.IntegrateKParallel = true;
  }
  /*==============================================*/
  // Function setting the integral to be the gauss_legendre
  // @args:
  // degree: the degree of gauss_legendre integral, default 1024
  /*==============================================*/
  void SimulationPlanar::optUseQuadgl(const int degree){
    degree_ = degree;
    options_.IntegralMethod = GAUSSLEGENDRE_;
  }
  /*==============================================*/
  // Function setting the integral to be the gauss_kronrod
  /*==============================================*/
  void SimulationPlanar::optUseQuadgk(){
    options_.IntegralMethod = GAUSSKRONROD_;
  }

  /*==============================================*/
  // This function gets the flux at a given kx
  // @args:
  // omegaIndex: the index of omega
  // KParallel: the KParallel value, normalized
  // NOTE:
  // assuming scalar or
  //  eps_x,  0,   0
  //    0  ,eps_x, 0
  //    0     0,  eps_z
  // make sure you understand your problem whether can be solved by this function
  /*==============================================*/
  double SimulationPlanar::getPhiAtKParallel(const int omegaIdx, const double KParallel){
    if(options_.IntegrateKParallel == false){
      std::cerr << "Cannot use kparallel integral here!" << std::endl;
      throw UTILITY::InternalException("Cannot use kparallel integral here!");
    }
    if(omegaIdx >= numOfOmega_){
      std::cerr << std::to_string(omegaIdx) + ": out of range!" << std::endl;
      throw UTILITY::RangeException(std::to_string(omegaIdx) + ": out of range!");
    }
    if(curOmegaIndex_ != omegaIdx){
      curOmegaIndex_ = omegaIdx;
      this->buildRCWAMatrices();
    }
    return POW2(omegaList_[omegaIdx] / datum::c_0) / POW2(datum::pi) * KParallel *
      poyntingFlux(omegaList_[omegaIdx] / datum::c_0 / MICRON, thicknessListVec_, KParallel, 0, EMatrices_,
      grandImaginaryMatrices_, eps_zz_Inv_Matrices_, Gx_mat_, Gy_mat_,
      sourceList_, targetLayer_,1, options_.polarization);
  }


  /*==============================================*/
  // This function integrates kx assuming scalar or
  //  eps_x,  0,   0
  //    0  ,eps_x, 0
  //    0     0,  eps_z
  // make sure you understand your problem whether can be solved by this function
  /*==============================================*/
  void SimulationPlanar::integrateKParallel(){
    if(options_.IntegrateKParallel == false){
      std::cerr << "Cannot use kparallel integral here!" << std::endl;
      throw UTILITY::InternalException("Cannot use kparallel integral here!");
    }

    RCWAcMatricesVec EMatricesVec(numOfOmega_), grandImaginaryMatricesVec(numOfOmega_), eps_zz_Inv_MatricesVec(numOfOmega_);
    for(int i = 0; i < numOfOmega_; i++){
      curOmegaIndex_= i;
      this->buildRCWAMatrices();
      EMatricesVec[i] = EMatrices_;
      grandImaginaryMatricesVec[i] = grandImaginaryMatrices_;
      eps_zz_Inv_MatricesVec[i] = eps_zz_Inv_Matrices_;
    }
    #if defined(_OPENMP)
      #pragma omp parallel for num_threads(numOfThread_)
    #endif
    for(int i = 0; i < numOfOmega_; i++){
      ArgWrapper wrapper;
      wrapper.thicknessList = thicknessListVec_;
      wrapper.Gx_mat = Gx_mat_;
      wrapper.Gy_mat = Gy_mat_;
      wrapper.sourceList = sourceList_;
      wrapper.targetLayer = targetLayer_;
      wrapper.omega = omegaList_[i] / datum::c_0;
      wrapper.EMatrices = EMatricesVec[i];

      wrapper.grandImaginaryMatrices = grandImaginaryMatricesVec[i];
      wrapper.eps_zz_Inv = eps_zz_Inv_MatricesVec[i];
      wrapper.polar = options_.polarization;
      switch (options_.IntegralMethod) {
        case GAUSSLEGENDRE_:{
          Phi_[i] = gauss_legendre(degree_, wrapperFunQuadgl, &wrapper, kxStart_, kxEnd_);
          break;
        }
        case GAUSSKRONROD_:{
          double err;
          adapt_integrate(1, wrapperFunQuadgk, &wrapper, 1, &kxStart_, &kxEnd_, 0, ABSERROR, RELERROR, &Phi_[i], &err);
          break;
        }
        default:{
          break;
        }
      }
      Phi_[i] *= POW3(omegaList_[i] / datum::c_0) / POW2(datum::pi);
    }
  }

  /*==============================================*/
  // Implementaion of the class on 1D grating simulation
  /*==============================================*/
  SimulationGrating::SimulationGrating() : Simulation(){
    prefactor_ = 1;
    dim_ = ONE_;
  }
  /*==============================================*/
  // This is a thin wrapper for the usage of smart pointer
  /*==============================================*/
  Ptr<SimulationGrating> SimulationGrating::instanceNew(){
    return new SimulationGrating();
  }
  /*==============================================*/
  // This function add grating to a layer
  // @args:
  // layerName: the name of the layer
  // materialName: the name of the material
  // center: the center of the grating
  // width: the width of the grating
  /*==============================================*/
  void SimulationGrating::setLayerPatternGrating(
    const std::string layerName,
    const std::string materialName,
    const double center,
    const double width
  ){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(layerName) == layerInstanceMap_.cend()){
      std::cerr << layerName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(layerName + ": Layer does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(layerName)->second;
    layer->addGratingPattern(material, center, width);
  }
  /*==============================================*/
  // function setting the periodicity
  // p1: the length of one periodicity
  /*==============================================*/
  void SimulationGrating::setLattice(const double p1){
    lattice_.bx[0] = p1;
    lattice_.area = p1;

    reciprocalLattice_.bx[0] = 2 * datum::pi / p1;
    reciprocalLattice_.area = 2 * datum::pi / p1;
    structure_->setLattice(lattice_);
  }
  /*==============================================*/
  // function using adaptive resolution algorithm
  /*==============================================*/
  void SimulationGrating::optUseAdaptive(){
    options_.FMMRule = SPATIALADAPTIVE_;
  }
  /*==============================================*/
  // Implementaion of the class on 2D patterning simulation
  /*==============================================*/
  SimulationPattern::SimulationPattern() : Simulation(){
    prefactor_ = 1;
    dim_ = TWO_;
  }
  /*==============================================*/
  // This function add rectangle pattern to a layer
  // @args:
  // layerName: the name of the layer
  // materialName: the name of the material
  // centerx: the center of the rectangle in x direction
  // centery: the center of the rectangle in y direction
  // angle: the rotated angle with respect to x axis
  // widthx: the width of the rectangle in x direction
  // widthy: the width of the rectangle in y direction
  /*==============================================*/
  void SimulationPattern::setLayerPatternRectangle(
    const std::string layerName,
    const std::string materialName,
    const double centerx,
    const double centery,
    const double angle,
    const double widthx,
    const double widthy
  ){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(layerName) == layerInstanceMap_.cend()){
      std::cerr << layerName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(layerName + ": Layer does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(layerName)->second;
    double arg1[2] = {centerx, centery};
    double arg2[2] = {widthx, widthy};
    layer->addRectanlgePattern(material, arg1, angle, arg2);
  }
  /*==============================================*/
  // This function add circle pattern to a layer
  // @args:
  // layerName: the name of the layer
  // materialName: the name of the material
  // centerx: the center of the circle in x direction
  // centery: the center of the circle in y direction
  // radius: the radius of the circle
  /*==============================================*/
  void SimulationPattern::setLayerPatternCircle(
    const std::string layerName,
    const std::string materialName,
    const double centerx,
    const double centery,
    const double radius
  ){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(layerName) == layerInstanceMap_.cend()){
      std::cerr << layerName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(layerName + ": Layer does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(layerName)->second;
    double arg1[2] = {centerx, centery};
    layer->addCirclePattern(material, arg1, radius);
  }
  /*==============================================*/
  // This function add ellipse pattern to a layer
  // @args:
  // layerName: the name of the layer
  // materialName: the name of the material
  // centerx: the center of the ellipse in x direction
  // centery: the center of the ellipse in y direction
  // angle: the rotated angle with respect to x axis
  // halfwidthx: the halfwidth of the ellipse in x direction
  // halfwidthy: the halfwidth of the ellipse in y direction
  /*==============================================*/
  void SimulationPattern::setLayerPatternEllipse(
    const std::string layerName,
    const std::string materialName,
    const double centerx,
    const double centery,
    const double angle,
    const double halfwidthx,
    const double halfwidthy
  ){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(layerName) == layerInstanceMap_.cend()){
      std::cerr << layerName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(layerName + ": Layer does not exist!");
      return;
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(layerName)->second;
    double arg1[2] = {centerx, centery};
    double arg2[2] = {halfwidthx, halfwidthy};
    layer->addEllipsePattern(material, arg1, angle, arg2);
  }
  /*==============================================*/
  // This function add polygon pattern to a layer
  // @args:
  // layerName: the name of the layer
  // materialName: the name of the material
  // centerx: the center of the ellipse in x direction
  // centery: the center of the ellipse in y direction
  // angle: the rotated angle with respect to x axis
  // edgePoints: the vertices in counter clockwise order
  // numOfPoints: the number of vertices
  /*==============================================*/
  void SimulationPattern::setLayerPatternPolygon(
    const std::string layerName,
    const std::string materialName,
    const double centerx,
    const double centery,
    const double angle,
    double**& edgePoints,
    const int numOfPoint
  ){
    if(materialInstanceMap_.find(materialName) == materialInstanceMap_.cend()){
      std::cerr << materialName + ": Material does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(materialName + ": Material does not exist!");
      return;
    }
    if(layerInstanceMap_.find(layerName) == layerInstanceMap_.cend()){
      std::cerr << layerName + ": Layer does not exist!" << std::endl;
      throw UTILITY::IllegalNameException(layerName + ": Layer does not exist!");
      return;
    }
    if(numOfPoint < 3){
      std::cerr << "Needs no less than 3 vertices!" << std::endl;
      throw UTILITY::RangeException("Needs no less than 3 vertices!");
    }
    Ptr<Material> material = materialInstanceMap_.find(materialName)->second;
    Ptr<Layer> layer = layerInstanceMap_.find(layerName)->second;
    double arg1[2] = {centerx, centery};
    layer->addPolygonPattern(material, arg1, angle, edgePoints,numOfPoint);
  }
  /*==============================================*/
  // function setting the lattice
  // xLen: the length of coordinate in x direction
  // yLen: the length of coordinate in y direction
  // angle: the angle between the two vectors
  /*==============================================*/
  void SimulationPattern::setLattice(const double xLen, const double yLen, const double angle){
    // point1 = (x1,x2) = (xLen, 0), point2 = (y1,y2) = (yLen * cos(theta), yLen * sin(theta))
    // b1 = (2*pi/x1, -2*pi/y2 * y1), b2 = (2*pi/y2, 0)
    if(angle == 0.0 || angle == 180.0){
      std::cerr << "the angle should be within range of (0, 180), exclusive!" << std::endl;
      UTILITY::RangeException("the angle should be within range of (0, 180), exclusive!");
    }
    lattice_.bx[0] = xLen;
    lattice_.by[0] = yLen * cos(angle * datum::pi / 180);
    lattice_.by[1] = yLen * sin(angle * datum::pi / 180);
    lattice_.angle = angle;
    lattice_.area = lattice_.bx[0] * lattice_.by[1];

    reciprocalLattice_.bx[0] = 2 * datum::pi / lattice_.bx[0];
    reciprocalLattice_.bx[1] = -2 * datum::pi / (lattice_.by[1] * lattice_.bx[0]) * lattice_.by[0];
    reciprocalLattice_.by[1] = 2 * datum::pi / lattice_.by[1];
    reciprocalLattice_.angle = 180 - angle;
    reciprocalLattice_.area = std::abs(reciprocalLattice_.by[1] * reciprocalLattice_.bx[0]);
    structure_->setLattice(lattice_);
  }
  /*==============================================*/
  // function return the reciprocal lattice
  // @args:
  // lattice: the return reciprocal lattice
  /*==============================================*/
  void SimulationPattern::getReciprocalLattice(double lattice[4]){
    lattice[0] = reciprocalLattice_.bx[0];
    lattice[1] = reciprocalLattice_.bx[1];
    lattice[2] = reciprocalLattice_.by[0];
    lattice[3] = reciprocalLattice_.by[1];
  }
  /*==============================================*/
  // This is a thin wrapper for the usage of smart pointer
  /*==============================================*/
  Ptr<SimulationPattern> SimulationPattern::instanceNew(){
    return new SimulationPattern();
  }

}