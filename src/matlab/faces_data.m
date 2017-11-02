function [output,t]=faces_data(nids,train,test,input)
    tic;
    fprintf('0000');

    output.L=zeros(train*nids,1);
    output.A=zeros(train*nids,size(input.A,2));
    output.I=cell(train*nids,1);
    output.F{1}=zeros(train*nids,size(input.F{1},2));
    output.F{2}=zeros(train*nids,size(input.F{2},2));
    output.eval.L=zeros(test*nids,1);
    output.eval.F{1}=zeros(test*nids,size(input.F{1},2));
    output.eval.F{2}=zeros(test*nids,size(input.F{2},2));

    if nids<size(input.U,1),
        range=sort(randperm(size(input.U,1),nids));
    else
        range=1:size(input.U,1);
    end

    % for each identity
    for i=1:length(range),
        % find corresponding indexes
        idx=find(input.L==input.U(range(i)));
        d=size(idx,1);

        % extract corresponding features
        f1=input.F{1}(idx,:);
        f2=input.F{2}(idx,:);

        % compute inner distances
        d1=zeros(d,d);
        d2=zeros(d,d);

        for j=1:d,
            for k=j+1:d,
                d1(j,k)=norm(f1(j,:)-f1(k,:));
                d2(j,k)=norm(f2(j,:)-f2(k,:));
            end
        end

        % build the distance matrices
        d1=d1+d1';
        d2=d2+d2';

        % extract minimizers of the sum of distances
        j=(train*(i-1)+1):(train*i);
        k=1:train;
        
        l=(test*(i-1)+1):(test*i);
        m=(d-test+1):d;

        [~,idx]=sort(sum(d1));
        output.F{1}(j,:)=f1(idx(k),:);
        output.eval.F{1}(l,:)=f1(idx(m),:);

        [~,idx]=sort(sum(d2));
        output.F{2}(j,:)=f2(idx(k),:);
        output.eval.F{2}(l,:)=f2(idx(m),:);

        % store labels, attrributes and images
        output.L(j)=repmat(input.U(range(i)),train,1);
        output.eval.L(l)=repmat(input.U(range(i)),test,1);
        output.A(j,:)=input.A(j,:);
        output.I(j)=input.I(j);
        
        fprintf('\b\b\b\b%04d',i);
    end

    fprintf('\b\b\b\b');
    t=toc;
end