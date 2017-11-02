close all;
warning('off','images:initSize:adjustingMag');

fprintf('[+] faces similartities\n |- start time: %s\n',datestr(now));

% script data
main.nids=2343;
main.train=1;
main.test=10;
main.action='output';
main.dpath='../../workspace/lending-solutions/data/09_celeba/';
main.ipath=fullfile(main.dpath,'features');
main.prefix='30_';
main.suffix='_cropped';
main.opath=fullfile(main.ipath,sprintf('%s%02d%s',main.prefix,main.train,main.suffix));

% load data
if isempty(main.action) && ~exist('data','var'),
    if  7~=exist(main.opath,'dir'),
        main.action=1;
        mkdir(main.opath);
        fprintf(' |- data file not found, forcing recomputation.\n');
    else
        tic;
        fprintf(' |- loading data... ');
        
        vars={'L','I','F','A','X','C','eval'};
        files={'labels','images','features','attributes','embeddings','clusters','evaluation'};

        for i=1:numel(vars),
            fpath=fullfile(main.opath,[files{i} '.mat']);
            if 2==exist(fpath,'file'),
                tmp=load(fpath);
                data.(vars{i})=tmp.(vars{i});
            else
                data.(vars{i})={};
                main.action=1;
            end
        end        
        
        fprintf('done! in %f seconds.\n',toc);
    end
end

% load inputs
if strcmp('data',main.action) && ~exist('input','var'),
    fprintf(' |- loading input data... ');
    [input,t]=faces_input(main.ipath,main.prefix,main.suffix);
    fprintf('done! in %f seconds.\n',t);
end

switch main.action,
    case 'data', % (re)compute base data?
        fprintf(' |- generating base data... ');
        [data,t]=faces_data(main.nids,main.train,main.test,input);
        fprintf('done! in %f seconds.\n',t);

        fprintf(' |- storing base data... ');
        tic;
        save(fullfile(main.opath,'images.mat'),'-struct','data','I','-v7.3');
        save(fullfile(main.opath,'labels.mat'),'-struct','data','L','-v7.3');
        save(fullfile(main.opath,'attributes.mat'),'-struct','data','A','-v7.3');
        save(fullfile(main.opath,'features.mat'),'-struct','data','F','-v7.3');
        save(fullfile(main.opath,'evaluation.mat'),'-struct','data','eval','-v7.3');
        fprintf('done! in %f seconds.\n',toc);

    case 'embeddings', % (re)compute embeddings?
        % tsne parameters
        tsne.dims=3;
        tsne.perplexity=10;
        tsne.theta=0.631;
        tsne.iterations=500;

        fprintf(' |- fc7 embeddings with tsne... ');
        [data.X{1},t]=faces_tsne(data.F{1},tsne);
        fprintf('done! in %f seconds.\n',t);

        fprintf(' |- fc8 embeddings with tsne... ');
        [data.X{2},t]=faces_tsne(data.F{2},tsne);
        fprintf('done! in %f seconds.\n',t);

        fprintf(' |- storing embeddings... ');
        tic;
        save(fullfile(main.opath,'embeddings.mat'),'-struct','data','X','-v7.3');
        fprintf('done! in %f seconds.\n',toc);

    case 'clusters', % (re)compute clusters?
        % clustering parameters
        K=round([1 0.75 0.5 0.25]*main.nids);
        clustering.empty='drop';
        clustering.maxiter=300;

        data.C={};
        I=cellfun(@(x)fullfile(main.dpath,x),data.I,'uni',false);

        for i=1:length(K),
            clustering.k=K(i);
            clustering.path=sprintf('clusters_%04d',K(i));
            if 7~=exist(fullfile(main.opath,clustering.path),'dir'),
                mkdir(fullfile(main.opath,clustering.path));
            end
            fprintf(' |-[+] clustering (kmeans, k=%d)\n',K(i));

            fprintf(' |  |- fc7 clusters... ');
            [idx,centroids,t]=faces_kmeans(data.F{1},clustering);
            data.C{i,1}.k=size(centroids,1);
            data.C{i,1}.idx=idx;
            data.C{i,1}.centroids=centroids;
            fprintf('done! in %f seconds.\n',t);

            fprintf(' |  |- fc7 mean images... ');
            impath=fullfile(clustering.path,sprintf('fc7_%%06d.png'));
            [data.C{i,1}.images,t]=faces_mean(data.C{i,1},I,main.opath,impath);
            fprintf('done! in %f seconds.\n',t);

            fprintf(' |  |- fc8 clusters... ');
            [idx,centroids,t]=faces_kmeans(data.F{2},clustering);
            data.C{i,2}.k=size(centroids,1);
            data.C{i,2}.idx=idx;
            data.C{i,2}.centroids=centroids;
            fprintf('done! in %f seconds.\n',t);

            fprintf(' |  ''- fc8 mean images... ');
            impath=fullfile(clustering.path,sprintf('fc8_%%06d.png'));
            [data.C{i,2}.images,t]=faces_mean(data.C{i,2},I,main.opath,impath);
            fprintf('done! in %f seconds.\n',t);
        end
        
        fprintf(' |- storing clustering data... ');
        save(fullfile(main.opath,'clusters.mat'),'-struct','data','C','-v7.3');
        fprintf('done! in %f seconds.\n',toc);

    case 'output', % show some results
        plot.nids=main.nids;
        plot.count=main.train;
        plot.mode='clusters-test';
        plot.clear=50;
        plot.dot=30;
        plot.colors=[]; % computed by faces_results(...)

        plot=faces_results(data,plot,main.opath);
end

fprintf(' ''- end time: %s\n',datestr(now));

% ready for plotting!
S=load('gong');
sound(S.y,S.Fs)

clearvars -except data input