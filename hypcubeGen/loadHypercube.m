function [hypercube,parameters]   = loadHypercube( inputFilePath, plotBands=0 )
  %=============================================================================
  % Developed:  March 25, 2019
  % Autor:      Jairo Salazar Vázquez
  %             jairosalazarvazquez@gmail.com
  % Note:       Tested in Octave 4.2.2
  % INPUT:      
  %             inputFilePath:
  %               A string with the path to a hypercube exported with the 
  %               software Calibration HypCam Beta 1.0
  %             plotBands:
  %               0: Not Plot Required | 1: Plot 25 wavebands in a 5x5 grid
  % OUTPUT:
  %             hypercube:
  %               A 3D matrix with Rows x Cols X Wavebands containing a exported
  %               hypercube with the software Calibration HypCam Beta 1.0
  %             parameters: 
  %               An structure describing the hypercube parameter with the 
  %               fields: creationDate, Columns, Rows, L (Number of Wavebands), 
  %               and Wavelengths (An array of length L containing one  
  %               wavelength in nanometers for each waveband).
  %=============================================================================
  if ~exist(inputFilePath,'file')
    fprintf("\nFile not exists\n\n");
  else
    fileObject                    = fopen(inputFilePath);
    % Get Hypercube's Parameters
    parameters.creationDate       = "";
    parameters.creationDate       = getCSVField(fileObject,0);
    parameters.Columns            = getCSVField(fileObject);
    parameters.Rows               = getCSVField(fileObject);
    parameters.L                  = getCSVField(fileObject);
    % Read Wavelengths
    parameters.Wavelengths        = zeros(1,parameters.L);
    for l=1:parameters.L
        parameters.Wavelengths(l) = getCSVField(fileObject);
    end
    % Read Hypercube
    hypercube                     = zeros(parameters.Rows,parameters.Columns,parameters.L);
    for l=1:parameters.L
      for r=1:parameters.Rows
        for c=1:parameters.Columns          
          hypercube(r,c,l)        = getCSVField(fileObject);
        end
      end
      fprintf("Loading waveband %d of %d \n",l,parameters.L);
      fflush(stdout);
    end
    % Read last 
  end
  % Plot Imported Hypercube
  if plotBands
    numImgs = 25;
    if(parameters.L<numImgs)
      n     = parameters.L;
      lstL  = 1:n;
    else
      n     = numImgs;
      lstL  = round( 1:(parameters.L/n):parameters.L );
    end
    rc = ceil(sqrt(n));
    for l=1:length(lstL)
      subplot(rc,rc,l);
      imshow(hypercube(:,:,l),[]);
      title(num2str( parameters.Wavelengths( lstL(l) ) ));
    end
  end
  
  fprintf("Hypercube Loaded Successfully \n");
  fflush(stdout);
  
function newField     = getCSVField(fileObject,isNumber=1,separator=',')
  %=============================================================================
  % This function reds a field separated by [separator] from an object type 
  % file.
  % INPUNT: 
  %   fileObject:
  %     Object created with the function "fopen"
  %   isNumber:
  %     0: if the expected field is text | 1: if the expected field is number
  %   separator:
  %     Reference separator, usually: ","
  % OUTPUT:
  %   newField:
  %     An field separated by te caracter [separator] type text or number, 
  %     according to the function's parameter [isNumber]
  %=============================================================================
  newField            = "";
  while( tmpCharacter  = fgets(fileObject,1) ) != -1
    if tmpCharacter == ','
      break;
    else
      newField        = [newField tmpCharacter];
    end 
  end
  if strcmp(newField,"")
    newField          = -1;
  end
  if isNumber
    newField          = str2double(newField);
  end
  
  
  
  
  
  