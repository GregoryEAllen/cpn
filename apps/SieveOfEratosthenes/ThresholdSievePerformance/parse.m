clear data fid line sizex sizey;
fid = fopen('results.txt', 'r');

data = [];
while (feof(fid) == 0)
   line = fgetl(fid);
   line = ['[' line ']'];
   data = [data; parse_json(line)];
   
end

fclose(fid);

[sizex, sizey] = size(data);
count = 0;
lookup = [];
X = cell(1);
Y = cell(1);
for i = 1:sizex
    maxprime = data{i, 1};
    time = data{i, 7};
    x = log10(maxprime);
    %y = time/log10(maxprime);
    y = log10(maxprime/time);
    queuesize = data{i, 2};
    threshold = data{i, 3};
    wheel = data{i, 4};
    ppf = data{i, 5};
    zcopy = data{i, 6};
    column = [queuesize; threshold; wheel; ppf{1}; ppf{2}; ppf{3}; zcopy; 0];
    if (numel(lookup) == 0)
        count = 1;
        index = 1;
        column(8) = index;
        lookup = column;
        X{1} = x;
        Y{1} = y;
    else
        found = -1;
        for j = 1:size(lookup,2)
            comp = lookup(:,j) == column;
            f = find(comp == 0);
            if (numel(f) == 1)
                if (f == 8)
                    found = j;
                    break;
                end
            end 
        end
        if (found == -1)
            count = count + 1;
            index = count;
            column(8) = index;
            lookup = [lookup column];
            X = [X x];
            Y = [Y y];
        else
            index = lookup(8,found);
            X{index} = [ X{index} x];
            Y{index} = [ Y{index} y];
        end
    end
end
%%
Xp = cell(size(X));
Yp = cell(size(Y));
for i = 1:size(X,2)
    Xp{i} = unique(X{i});
    Yp{i} = zeros(size(Xp{i}));
    for j = 1:size(Xp{i},2)
        inds = find(X{i} == Xp{i}(j));
        Yp{i}(j) = min(Y{i}(inds));
    end
end
%%
dots = ['o' 'x' '^' 'v' '*' '>' '<'];
figure
hold on
for i = 1:size(X,2)
    %scatter(X{i}, Y{i}, dots(i))
    plot(Xp{i}, Yp{i})
end
hold off
clear fid line sizex sizey;

