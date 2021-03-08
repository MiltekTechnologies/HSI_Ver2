%=============================================================================
% Developed:  March 25, 2019
% Autor:      Jairo Salazar Vázquez
%             jairosalazarvazquez@gmail.com
% Note:       Tested in Octave 4.2.2
% INPUT:      
%             User selects the .hypercube to be imported
% OUTPUT:
%             This script exports the hypercube as a .mat file
%=============================================================================

clear; close all;
clc;

%-----------------------------------------------
% PARAMETERS
%-----------------------------------------------
normalizedBy    = 0;     % 0: Raw | 1: By band | 2: By cube 
amplifyVal      = 1;     % A positive scalar 

%-----------------------------------------------
% LOAD HYPERCUBE
%-----------------------------------------------
[fname, inputPath, fltidx]    = uigetfile();
if 1 %Select a ".hypercube"
  originPath                  = [inputPath fname];
  destinePath                 = [inputPath strrep(fname,'.hypercube','') '.mat'];
  [hypercube,parameters]      = loadHypercube( originPath );
  H                           = hypercube;
  save(destinePath, '-mat7-binary', 'H', 'parameters');
  originPath
else % Select a ".mat"
  destinePath                 = [inputPath fname]; 
  load(destinePath);
end


%-----------------------------------------------
% NORMALIZE
%-----------------------------------------------
H               = hypercube;
[R,C,L]         = size(H);

% Transform to Grayscale
H               = H / 255.0;
H(find(H>1.0))  = 1.0;

% Raw data amplified
if normalizedBy == 0
  H             = H * amplifyVal;
  cubeMinVal    = min(min(min(H)));
  cubeMaxVal    = max(max(max(H)));
end
% Normalized by band
if normalizedBy == 1
  for l=1:L
    tmpImg      = squeeze( H(:,:,l) );
    minVal      = min(min(tmpImg));
    maxVal      = max(max(tmpImg));
    tmpImg      = ( (tmpImg-minVal) / (maxVal-minVal) ) * amplifyVal;
    H(:,:,l)    = tmpImg;
  end
end
% Normalized by whole cube
if normalizedBy == 2
  cubeMinVal    = min(min(min(H)));
  cubeMaxVal    = max(max(max(H)));
  H             = (H-cubeMinVal) / (cubeMaxVal-cubeMinVal);
  H             = H * amplifyVal;
end


%-----------------------------------------------
% PLOT HYPERCUBE
%-----------------------------------------------
if 0
  for l=1:L
    tmpImg      = squeeze( H(:,:,l) );
    figure, imshow(tmpImg);
    title(num2str(parameters.Wavelengths(l)));
  end
end

%-----------------------------------------------
% SAVE HYPERCUBE
%-----------------------------------------------
if 1
  % Clear Directory
  dirname         = strrep(destinePath,".mat","");
  dirname         = [dirname '/'];
  if isdir( dirname )
    dinfo = dir(dirname);
    dinfo([dinfo.isdir]) = [];   %skip directories
    filenames = fullfile(dirname, {dinfo.name});
    if length(filenames) > 0
      delete( filenames{:} );
    end
  else
    mkdir(dirname);
  end  
  % Export Wavebands
  for l=1:L
    tmpImg        = squeeze( H(:,:,l) );
    tmpImg        = tmpImg / max(max(tmpImg));
    tmpFilename   = [dirname '/' num2str(round(parameters.Wavelengths(l))) '.png'];
    imwrite(tmpImg, tmpFilename, 'png');
    %fprintf("%d of %d \n",l,L);
  end
  fprintf("\n\nCube has been exported\n\n");
end





