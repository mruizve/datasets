function [mean,t]=faces_mean(C,I,ipath,impath)
    tic
    fprintf('0000');

    mean=cell(C.k,1);
    
    % compute mean image of each cluster
    for i=1:C.k,
        images=I(C.idx==i);

        mean{i}=im2double(imread(images{1}));
        for j=2:numel(images),
            mean{i}=mean{i}+im2double(imread(images{j}));
        end
        mean{i}=mean{i}/numel(images);

        imname=sprintf(impath,i);
        imwrite(mean{i},fullfile(ipath,imname));
        mean{i}=imname;
        
        fprintf('\b\b\b\b%04d',i);
    end

    fprintf('\b\b\b\b');
    t=toc;
end