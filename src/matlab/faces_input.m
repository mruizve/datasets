function [output,t]=faces_input(dataset,prefix,suffix)
    tic;
    
    % load labels and attributes
    load(fullfile(dataset,[prefix 'labels.mat']));
    load(fullfile(dataset,[prefix 'attributes.mat']));

    % read attributes names
    file=fopen(fullfile(dataset,[prefix 'attributes_names.txt']),'r');
    names=textscan(file,'%s');
    names=names{1};
    fclose(file);

    % read images paths
    file=fopen(fullfile(dataset,[prefix 'images.txt']),'r');
    images=textscan(file,'%s');
    images=images{1};
    fclose(file);

    % load vgg fc7 features
    load(fullfile(dataset,[prefix 'vgg_fc7' suffix '.mat']));
    F1=features;

    % load vgg fc8 features
    load(fullfile(dataset,[prefix 'vgg_fc8' suffix '.mat']));
    F2=features;

    % sort labels, attributes and features
    [output.L,idx]=sort(labels);

    % sort attributes, features and images paths
    output.A=attributes(idx,:);
    output.F{1}=F1(idx,:);
    output.F{2}=F2(idx,:);
    output.I=images(idx);

    % extract unique labels
    output.U=unique(output.L);

    t=toc;
end