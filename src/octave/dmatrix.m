% distance matrix computation for big dataset
function D=dmatrix( labels, features )
	% validate input arguments
	narginchk(2,2)

	% sort labels
	[L,I]=sort(labels);

	% sort features
	F=features(I,:);

	% count number of occurrences of each label
	uL=unique(L);
	count=histc(L(:),uL);
	indexes=[0; cumsum(count)];

	% initialize the output matrix
	m=size(uL,1);
	D=zeros(m,m,3);

	% for each label pair
	for i=1:numel(uL),
		for j=i:numel(uL),
			fprintf('%06d,%06d\b\b\b\b\b\b\b\b\b\b\b\b\b',i,j);

			% compute the mean, max and min distance between their features vectors
			d_mean=0;
			d_max=0;
			d_min=1e9;

			% distance are computed between all images of the labels pair
			for k=1:count(i),
				for l=1:count(j),
					% compute indexes of the sorted features matrix
					ii=indexes(i)+k;
					jj=indexes(j)+l;

					% computer features distance
					d=norm(F(ii,:)-F(jj,:));
				
					% accumulate mean
					d_mean=d_mean+d;

					% get maximum distance
					if( d>d_max ),
						d_max=d;
					end

					% get minimum distance
					if( d<d_min ),
						d_min=d;
					end
				end
			end

			% compute mean distance
			d_mean=d_mean/(count(i)*count(j));

			% update the values of the output matrix
			D(i,j,1)=d_mean;
			D(i,j,2)=d_max;
			D(i,j,3)=d_min;
		end
	end
	fprintf('\n');
end
