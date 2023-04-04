void readTree() {

    TFile* inputFile = new TFile("FileName.root", "READ");

    TTree* inputTree = inputFile->Get<TTree>("TreeName");

    //Branch simple que no es un array
    float variableF;

    //El tamaño del array daselo más grande que el número de entradas
    float variableArrayF[500];

    //Para cada Branch hay que asociar la direccion de memoria a una variable. En el caso del array con pasar la variable basta. Es importante que el tipo de la variable o el array (en este caso float) coincide con el tipo guardado en la branch, se puede buscar en el TBrowser
    inputTree->SetBranchAddress("BranchNameNOArray",&variableF);
    inputTree->SetBranchAddress("BranchNameArray",variableArrayF);
    
    Long64_t maxEntires = inputTree->GetEntries();
    for(Long64_t iEntry = 0; iEntry < maxEntries; iEntry++) {
      inputTree->GetEntry(iEntry);
      
      //Do stuff with your variables

      std::cout << variableF << std::endl //Por ejemplo
    }

    delete inputTree;
      
    inputFile->Close();
    delete inputFile;

}
