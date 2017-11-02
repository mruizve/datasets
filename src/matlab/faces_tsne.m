function [mapped,t]=faces_tsne(F,tsne)
    tic;

    X=bsxfun(@minus, F, mean(F,1));
    X=X/max(X(:));

    tsne_path = which('fast_tsne');
    tsne_path = fileparts(tsne_path);
    
    % Compile t-SNE C code
    if(~exist(fullfile(tsne_path,'./bh_tsne'),'file') && isunix)
        system(sprintf('g++ %s %s -o %s -O2',...
            fullfile(tsne_path,'./sptree.cpp'),...
            fullfile(tsne_path,'./tsne.cpp'),...
            fullfile(tsne_path,'./bh_tsne')));
    end

    % run the fast diffusion SNE implementation
    write_data(X,tsne.dims,tsne.theta,tsne.perplexity,tsne.iterations);
    [flag, cmdout] = system(['"' fullfile(tsne_path,'./bh_tsne') '"']);
    if(flag~=0)
        error(cmdout);
    end
    [mapped,~,~]=read_data;   

    delete('data.dat');
    delete('result.dat');

    t=toc;
end

% writes the datafile for the fast t-SNE implementation
function write_data(X, no_dims, theta, perplexity, max_iter)
    [n, d] = size(X);
    h = fopen('data.dat', 'wb');
	fwrite(h, n, 'integer*4');
	fwrite(h, d, 'integer*4');
    fwrite(h, theta, 'double');
    fwrite(h, perplexity, 'double');
	fwrite(h, no_dims, 'integer*4');
    fwrite(h, max_iter, 'integer*4');
    fwrite(h, X', 'double');
	fclose(h);
end

% reads the result file from the fast t-SNE implementation
function [X, landmarks, costs] = read_data
    h = fopen('result.dat', 'rb');
	n = fread(h, 1, 'integer*4');
	d = fread(h, 1, 'integer*4');
	X = fread(h, n * d, 'double');
    landmarks = fread(h, n, 'integer*4');
    costs = fread(h, n, 'double');      % this vector contains only zeros
    X = reshape(X, [d n])';
	fclose(h);
end
