function faces_plot(X,plot)
    switch size(X{1},2),
        case 2,
            subplot(1,2,1);
            scatter(X{1}(:,1),X{1}(:,2),plot.dot,plot.colors,'filled');
            axis tight;

            subplot(1,2,2);
            scatter(X{2}(:,1),X{2}(:,2),plot.dot,plot.colors,'filled');
            axis tight;

        case 3,
            subplot(1,2,1);
            scatter3(X{1}(:,1),X{1}(:,2),X{1}(:,3),plot.dot,plot.colors,'filled');
            axis tight;
            axis vis3d

            subplot(1,2,2);
            scatter3(X{2}(:,1),X{2}(:,2),X{2}(:,3),plot.dot,plot.colors,'filled');
            axis tight;
            axis vis3d
    end
end