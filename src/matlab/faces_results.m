function plot=faces_results(data,plot,opath)
    switch plot.mode,
        case 'identities-animation',
            ftitle=sprintf('identities animation (nids=%d, nimg=%d)',plot.nids,plot.count);
            figure('Name',ftitle,'NumberTitle','off')
            faces_animate(data.X,data.L,plot);

        case 'attributes',

        case 'identities',
            % generate scatter colors based on identity labels
            colors=distinguishable_colors(plot.nids);
            plot.colors=zeros(plot.count*plot.nids,3);
            for i=1:plot.nids,
                j=plot.count*(i-1)+1;
                aux=j+(plot.count-1);
                plot.colors(j:aux,:)=repmat(colors(i,:),plot.count,1);
            end

            % plot data
            figure('Name','labeled identities','NumberTitle','off')
            faces_plot(data.X,plot);

        case 'clusters-animation',
            figure('Name','clustering animation','NumberTitle','off')
            for i=numel(data.C):-1:1,
                switch size(data.C{i}.centroids,2),
                    case 4096,
                        ftitle='fc7';
                    case 2622,
                        ftitle='fc8';
                    otherwise,
                        error('faces:results','unknown feature size');
                end
                
                ftitle=['clustering animation (nids=%d, nimg=%d, k=%d, ' ftitle ')'];
                ftitle=sprintf(ftitle,plot.nids,plot.count,data.C{i}.k);
                set(gcf,'Name',ftitle);
                faces_animate(data.X,data.C{i}.idx,plot);
            end

        case 'clusters-immean',
            figure(1);
            figure(2);
            for i=1:numel(data.C(:,1)),
                cpath=fullfile(opath,sprintf('clusters_%04d',data.C{i,1}.k));
                
                range=randperm(numel(data.C{i,1}.images),72);

                image1=[];
                image2=[];
                aux1=imread(sprintf(fullfile(cpath,'fc7_%06d.png'),range(1)));
                aux2=imread(sprintf(fullfile(cpath,'fc8_%06d.png'),range(1)));
                for j=2:numel(range),
                    aux1=[aux1 imread(sprintf(fullfile(cpath,'fc7_%06d.png'),range(j)))];
                    aux2=[aux2 imread(sprintf(fullfile(cpath,'fc8_%06d.png'),range(j)))];
                    if ~mod(j,12),
                        image1=[image1; aux1];
                        image2=[image2; aux2];
                        aux1=[];
                        aux2=[];
                    end
                end
                
                figure(1);
                ftitle='clustering animation (nids=%d, nimg=%d, k=%d, fc7)';
                set(gcf,'Name',sprintf(ftitle,plot.nids,plot.count,data.C{i,1}.k));
                imshow(image1);

                figure(2);
                ftitle='clustering animation (nids=%d, nimg=%d, k=%d, fc8)';
                set(gcf,'Name',sprintf(ftitle,plot.nids,plot.count,data.C{i,1}.k));
                imshow(image2);

                pause;
            end

        case 'clusters-stats',
            for i=1:numel(data.C(:,1)),
                % get data labels (class and clusters)
                cl=unique(data.L);
                kl={unique(data.C{i,1}.idx); unique(data.C{i,2}.idx)};

                % identify number of classes and clusters
                nc=length(cl);
                nk=cellfun(@(e) length(e),kl,'uni',false);

                % contingency table computation
                T=cellfun(@(e) zeros(nc,e),nk,'uni',false);
                for j=1:nc,
                    % extract all elements of class cl(i)
                    idx=(data.L==cl(j));

                    % get corresponding clusters
                    for k=1:numel(T),
                        aux=data.C{i,k}.idx(idx);
                        x=unique(aux);
                        [y,~]=histc(aux,x);
                        T{k}(j,x)=y;
                    end
                end
                
                H=cell(numel(T),1);
                C=cell(numel(T),1);
                V=cell(numel(T),1);
                for j=1:numel(T)
                    % homogeneity computation
                    aux=T{j}./repmat(sum(T{j},2),1,size(T{j},2));
                    aux(aux==0)=1; % map to zero all zero elements
                    H_CK=-sum(sum(T{j}.*log(aux)./size(data.L,1)));
                    aux=sum(T{j},2)./nc;
                    H_C=-sum(sum(T{j},2).*log(aux)./nc);
                    
                    if 0==H_C,
                        H{j}=1;
                    else
                        H{j}=1-H_CK/H_C;
                    end
                    
                    % completeness computation
                    aux=T{j}./repmat(sum(T{j},1),size(T{j},1),1);
                    aux(aux==0)=1; % map to zero all zero elements
                    H_CK=-sum(sum(T{j}.*log(aux)./size(data.L,1)));
                    aux=sum(T{j},1)./nc;
                    H_C=-sum(sum(T{j},1).*log(aux)./nc);
                    
                    if 0==H_C,
                        C{j}=1;
                    else
                        C{j}=1-H_CK/H_C;
                    end

                    % VC-measure computation
                    beta=1;
                    V{j}=(1+beta)*H{j}*C{j}/(beta*H{j}+C{j});
                end
                
                fprintf('[nids=%d, nimg=%d, k=%04d, fc7] H=%f, C=%f, V-measure(beta=1)=%f\n',plot.nids,plot.count,data.C{i,1}.k,H{1},C{1},V{1});
                fprintf('[nids=%d, nimg=%d, k=%04d, fc8] H=%f, C=%f, V-measure(beta=1)=%f\n',plot.nids,plot.count,data.C{i,2}.k,H{2},C{2},V{2});
            end
            
        case 'clusters-test',
            L={zeros(size(data.eval.L)); zeros(size(data.eval.L))};
            for i=1:size(data.eval.L,1),
                dF=(data.F{1}-repmat(data.eval.F{1}(i,:),size(data.L,1),1)).^2;
                [~,idx]=min(sqrt(sum(dF,2)));
                L{1}(i)=data.L(idx);
                
                size(idx)
                size(L{1}(idx))

                dF=(data.F{2}-repmat(data.eval.F{2}(i,:),size(data.L,1),1)).^2;
                [~,idx]=min(sqrt(sum(dF,2)));
                L{2}(i)=data.L(idx);
                
                fprintf('%06d\n',i);
            end
            
            H=confussionmat(data.eval.L,L{1});
            imshow(H./max(H(:)));
            pause;

        otherwise,
            error('faces:plot',['unknown plot mode: ''' plot.mode '''']);
    end
end