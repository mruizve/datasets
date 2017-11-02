function faces_animate(X,L,plot)
    % compute data bounds
    mn1=min(X{1}); mx1=max(X{1});
    mn2=min(X{2}); mx2=max(X{2});

    % get labels
    U=unique(L);

    % generate color
    colors=distinguishable_colors(size(U,1));
    
    % vieweing angles
    az=45;
    el=45;

    for i=1:size(U,1),
        idx=find(L==U(i));
        plot.colors=zeros(size(L,1),3);
        plot.colors=repmat(colors(i,:),length(idx),1);

        if ~mod(i,plot.clear),
            clf;
        end
        az=az+2;
        
        switch size(X{1},2),
                case 2,
                    subplot(1,2,1);
                    hold on;
                    scatter(X{1}(idx,1),X{1}(idx,2),plot.dot,plot.colors,'filled');
                    hold off;
                    axis([mn1(1) mx1(1) mn1(2) mx1(2)]);

                    subplot(1,2,2);
                    hold on;
                    scatter(X{2}(idx,1),X{2}(idx,2),plot.dot,plot.colors,'filled');
                    hold off;
                    axis([mn2(1) mx2(1) mn2(2) mx2(2)]);

                case 3,
                    subplot(1,2,1);
                    hold on;
                    scatter3(X{1}(idx,1),X{1}(idx,2),X{1}(idx,3),plot.dot,plot.colors,'filled');
                    hold off;
                    axis([mn1(1) mx1(1) mn1(2) mx1(2) mn1(3) mx1(3)]);
                    view(az,el);
                    axis vis3d

                    subplot(1,2,2);
                    hold on;
                    scatter3(X{2}(idx,1),X{2}(idx,2),X{2}(idx,3),plot.dot,plot.colors,'filled');
                    hold off;
                    axis([mn2(1) mx2(1) mn2(2) mx2(2) mn2(3) mx2(3)]);
                    view(az,el);
                    axis vis3d
        end
        drawnow;
    end
end