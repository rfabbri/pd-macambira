function cipicdb2wav(src_path, output_path)
%CIPICDB2WAV Summary of this function goes here
%   src_path correspond to the root of extracted  archive containing
%   the CIPIC hrtf database which can be downloaded there
%   http://interface.idav.ucdavis.edu/data/CIPIC_hrtf_database.zip
%   
%   output_path is the directory where the converted database will
%   be stored, if the directory does not exists, it will be created

    mkdir(output_path)

    % get a list corresponding the the sets of measures to process
    pl = build_processing_list(src_path);

    % those value indexes are given in the cipic documentation,
    % they correspond to the azimuths and elevations available in the db
    azimuths = [-80 -65 -55 -45:5:45 55 65 80];
    elevations = -45 + 5.625*(0:49);

    % the external uses listen database formalism
    % see http://recherche.ircam.fr/equipes/salles/listen/
    % in listen database, angle(90,0) correspond to a point directly to the
    % left, and angle(-90,0) correspond to a point directely to the right
    % in CIPIC, this is inversed, meaning (90,0) correspond to right
    % and so on
    % consequenly, we intersed the azimuth angle label to correspond to
    % listen formalism
    azimuths = -1 * azimuths
    
    for ihrtf = 1:length(pl)
        'processing'
        fname = char(pl(ihrtf,1))
        outdir_name = [output_path '/' char(pl(ihrtf,2))]
        mkdir(outdir_name)
        data = load(fname)
        
        for azn=1:length(azimuths)
            for eln=1:length(elevations)
                
                az = azimuths(azn)
                el = elevations(eln)
                
                leftir = squeeze(data.hrir_l(azn, eln,:));
                rightir = squeeze(data.hrir_r(azn, eln,:));
                
                stereo(:,1) = leftir;
                stereo(:,2) = rightir;
                wav_name = sprintf('%s/%s_azim_%.3f_elev_%.3f.wav', outdir_name, char(pl(ihrtf,2)), az, el)
                wavwrite(stereo, 44100, 32, wav_name);
            end
        end
    end
    
%     out_dir='C:\\Users\\david\\Desktop\\\CIPIC_hrtf_database\\rudk_cipic_new';
%     
%     azimuths = [-80 -65 -55 -45:5:45 55 65 80];
%     elevations = -45 + 5.625*(0:49);
%     
%     for ifile = 1:nfiles
%         cur_dir = char(dir_list(ifile, :));
%         if cur_dir(1:8) == 'subject_'
%             disp(cur_dir)
%             cur_dir = strtrim(cur_dir);
%             cur_out = [out_dir '\\' cur_dir]
%             mkdir(cur_out);
%             load([cur_dir '\\\\hrir_final.mat']);
%             
%             % indexed by azimuth, elevation, and time
%             for iazi = 1:length(azimuths)
%                 for ielev = 1:length(elevations)
%                     left_data = hrir_l(iazi, ielev, :);
%                     right_data = hrir_r(iazi, ielev, :);
%                     [az, el] = normalize_az_el(azimuths(iazi), elevations(ielev));
%                     wav_name = sprintf('%s\\%s_az_%d_elev_%.2f.wav', cur_out, cur_dir, az, el)
%                     stereo_data(:,1) = left_data;
%                     stereo_data(:,2) = right_data;
%                     wavwrite(stereo_data, 44100, 32, wav_name);
%                     if abs(el - 90) < .01
%                         wav_name = sprintf('%s\\%s_az_%d_elev_%.2f.wav', cur_out, cur_dir, mod(az+180, 360), el);
%                         wavwrite(stereo_data, 44100, 32, wav_name);
%                     end
%                 end
%             end
%         end
%     end
%     
%     function [res_az, res_el] = normalize_az_el(az, el)
%         % elevation should be between - 90 and + 90
%         if el > 226 %% HACK
%             
%             'hack'
%             
%             el = -(el - 225);
%             az = az + 180;
%             
%         end
% 
%         if el > 90 + .005
%             el = 180 -el;
%             az = 180 + az;
%         end
%             
%             
%         az = mod(az, 360);
%         if (az < 0)
%             az = az + 360;
%         end
%         
%         res_az = az;
%         res_el = el;
    function res = build_processing_list(path)
        res = []
        c = {'tutu','tata','titi'}
        d = [c; {'tutu2','tata2','titi2'}]
        e = [d; {'tutu3','tata4','titi5'}]
        path
        l = dir([path '/standard_hrir_database/subject_*'])
        for i=1:length(l)
            fullname = [path '/standard_hrir_database/' l(i).name '/hrir_final.mat']
            res = [res; {fullname, char(l(i).name)}]
        end
% those files do not contains the same fields        
%         p = [path '/special_kemar_hrir/kemar_frontal/large_pinna_frontal.mat']
%         res = [res; {p, 'kemar_frontal_large_pinna'}]
%         p = [path '/special_kemar_hrir/kemar_frontal/small_pinna_frontal.mat']
%         res = [res; {p, 'kemar_frontal_small_pinna'}]
%         p = [path '/special_kemar_hrir/kemar_horizontal/large_pinna_final.mat']
%         res = [res; {p, 'kemar_horizontal_large_pinna'}]
%         p = [path '/special_kemar_hrir/kemar_horizontal/small_pinna_final.mat']
%         res = [res; {p, 'kemar_horizontal_small_pinna'}]
    end
end