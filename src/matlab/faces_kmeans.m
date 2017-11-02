function [idx,C,t]=faces_kmeans(F,cluster)
    tic;
    [idx,C]=kmeans(F,cluster.k,'EmptyAction',cluster.empty,'MaxIter',cluster.maxiter);
    t=toc;
end