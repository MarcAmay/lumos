%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% S Y S T E M  P A R A M E T E R S
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% room dimensions in meters
length = 10;
width = 10;
height = 5;
rho = 0.8;
m = 1;

% number of points per axis
num_points_x = 21;
num_points_y = 21;
num_points_z = 11;

power_transmitter = 1;
area_receiver = 0.0001;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% C O O R D I N A T E S  A N D  N O R M A L S
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% XYZ axes
X_axis = linspace(-length/2, length/2, num_points_x);
Y_axis = flip(linspace(-width/2, width/2, num_points_y));
Z_axis = linspace(0, -height, num_points_z);

X_axis_transmitter = linspace(-length/2, length/2, 7);
Y_axis_transmitter =  linspace(-width/2, width/2, 7);

% TRANSMITTER COORDINATES
coord_transmitter = [X_axis_transmitter(2) Y_axis_transmitter(6) 0; X_axis_transmitter(4) Y_axis_transmitter(6) 0; X_axis_transmitter(6) Y_axis_transmitter(6) 0;
    X_axis_transmitter(2) Y_axis_transmitter(4) 0; X_axis_transmitter(4) Y_axis_transmitter(4) 0; X_axis_transmitter(6) Y_axis_transmitter(4) 0;
    X_axis_transmitter(2) Y_axis_transmitter(2) 0; X_axis_transmitter(4) Y_axis_transmitter(2) 0; X_axis_transmitter(6) Y_axis_transmitter(2) 0];

% RECEIVER PLANE COORDINATES
coord_receiver = zeros(1,3);

for i = 1:num_points_y
    for j = 1: num_points_x
        coord_temp(1,1) = X_axis(j);
        coord_temp(1,2) = Y_axis(i);
        coord_temp(1,3) = Z_axis(num_points_z);
        coord_receiver = vertcat(coord_receiver, coord_temp);
    end
end
coord_receiver(1,:) = [];

% WALL COORDINATES
coord_wall = zeros(1,3);

for i = 1:num_points_y % cieling
    for j = 1: num_points_x
        coord_temp(1,1) = X_axis(j);
        coord_temp(1,2) = Y_axis(i);
        coord_temp(1,3) = 0;
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end

for i = 1:num_points_y % floor
    for j = 1: num_points_x
        coord_temp(1,1) = X_axis(j);
        coord_temp(1,2) = Y_axis(i);
        coord_temp(1,3) = Z_axis(num_points_z);
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end

for i = 1:num_points_z % right wall
    for j = 1: num_points_y
        coord_temp(1,1) = -X_axis(1);
        coord_temp(1,2) = Y_axis(j);
        coord_temp(1,3) = Z_axis(i);
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end

Y_axis = flip(Y_axis);
for i = 1:num_points_z % left wall
    for j = 1: num_points_y
        coord_temp(1,1) = X_axis(1);
        coord_temp(1,2) = Y_axis(j);
        coord_temp(1,3) = Z_axis(i);
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end

Y_axis = flip(Y_axis);
for i = 1:num_points_z % back wall
    for j = 1: num_points_x
        coord_temp(1,1) = X_axis(j);
        coord_temp(1,2) = Y_axis(1);
        coord_temp(1,3) = Z_axis(i);
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end

for i = 1:num_points_z % front wall
    for j = 1: num_points_x
        coord_temp(1,1) = X_axis(j);
        coord_temp(1,2) = -Y_axis(1);
        coord_temp(1,3) = Z_axis(i);
        coord_wall = vertcat(coord_wall, coord_temp);
    end
end
coord_wall(1,:) = [];

% NUMBER OF ELEMENTS
num_transmitter = size(coord_transmitter,1); 
num_receiver = num_points_x*num_points_y;
num_wallreflector = num_points_x*num_points_y*2 + num_points_z*num_points_y*2 + num_points_z*num_points_x*2;

% NORMAL
norm_transmitter = [0 0 -1];
norm_receiver = [0 0 1];
norm_wall = zeros(num_wallreflector,3);

for i = 1: num_points_x*num_points_y % ceiling
    norm_wall(i,:) = [0 0 -1];
    area_wall(i) = (length/num_points_x) * (width/num_points_y);
end

for i = (num_points_x*num_points_y+1): (num_points_x*num_points_y*2) % floor
    norm_wall(i,:)  = [0 0 1];
    area_wall(i) = (length/num_points_x) * (width/num_points_y);
end

for i = (num_points_x*num_points_y*2 + 1): (num_points_x*num_points_y*2 + num_points_z*num_points_y) % right
    norm_wall(i,:)  = [-1 0 0];
    area_wall(i) = (height/num_points_z) * (width/num_points_y);
end

for i = (num_points_x*num_points_y*2 + num_points_z*num_points_y + 1): (num_points_x*num_points_y*2 + num_points_z*num_points_y*2) % left
    norm_wall(i,:) = [1 0 0];
    area_wall(i) = (height/num_points_z) * (width/num_points_y);
end

for i = (num_points_x*num_points_y*2 + num_points_z*num_points_y*2 + 1): (num_points_x*num_points_y*2 + num_points_z*num_points_y*2 + num_points_z*num_points_x) % back
    norm_wall(i,:) = [0 -1 0];
    area_wall(i) = (height/num_points_z) * (length/num_points_x);
end

for i = (num_points_x*num_points_y*2 + num_points_z*num_points_y*2 + num_points_z*num_points_x + 1): (num_points_x*num_points_y*2 + num_points_z*num_points_y*2 + num_points_z*num_points_x*2) % front
    norm_wall(i,:) = [0 1 0];
    area_wall(i) = (height/num_points_z) * (length/num_points_x);
end


%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% L I N K  C O M P O N E N T S
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% TRANSMITTER TO RECEIVER / LOS
for i = 1: num_transmitter
    for j = 1: num_receiver
        [gain_LOS(i,j), distance_LOS(i,j), costheta_transmitter(i,j), costheta_receiver(i,j)] = gain(coord_transmitter(i,:), coord_receiver(j,:), norm_transmitter, norm_receiver, m, area_receiver);
    end
end

% WALL TO RECEIVER
for j = 1: num_receiver
    for k = 1: num_wallreflector
        [diffuse_walltoreceiver(j,k), distance_walltoreceiver(j,k), costheta_walltoreceiver(j,k), costheta_receiverfromwall(j,k)] = gain(coord_wall(k,:), coord_receiver(j,:), norm_wall(k,:), norm_receiver, m, area_receiver);
    end
end

% WALL TO WALL
for k = 1: num_wallreflector
    for l = 1: num_wallreflector
        [diffuse_walltowall(k,l), distance_walltowall(k,l), costheta_walltowall(k,l), costheta_wallfromwall(k,l)] = gain(coord_wall(k,:), coord_wall(l,:), norm_wall(k,:), norm_wall(l,:), m, area_wall(l));
    end
end

% TRANSMITTER TO WALL
for i = 1: num_transmitter
    for l = 1: num_wallreflector
        [diffuse_transmittertowall(i,l), distance_transmittertowall(i,l), costheta_transmittertowall(i,l), costheta_wallfromtransmitter(i,l)] = gain(coord_transmitter(i,:), coord_wall(l,:), norm_transmitter, norm_wall(l,:), m, area_wall(l));
    end
end

% DIFFUSE
diffuse_walltowall(isnan(diffuse_walltowall)) = 0;
diffuse_transmittertowall(isnan(diffuse_transmittertowall)) = 0;
diffuse_walltoreceiver(isnan(diffuse_walltoreceiver)) = 0;
rho_diagonal = diag(zeros(1, num_wallreflector) + rho);
identity = eye(num_wallreflector);

for i = 1: num_transmitter
    for j = 1: num_receiver
        gain_diffuse(i,j) = diffuse_walltoreceiver(j,:) * rho_diagonal * (inv(identity - diffuse_walltowall * rho_diagonal)) * (diffuse_transmittertowall(i,:))';
    end
end

gain_LOS_total = sum(gain_LOS);
gain_diffuse_total = sum(gain_diffuse);
gain_total = gain_LOS + gain_diffuse;
gain_total_total = sum(gain_total);


%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% P L O T
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
LOS_3D = reshape(gain_LOS_total, sqrt(num_receiver), sqrt(num_receiver))';
figure
surf(LOS_3D)
hold
axis([1 num_points_x 1 num_points_y 0 11e-6])

diffuse_3D = reshape(gain_diffuse_total, sqrt(num_receiver), sqrt(num_receiver))';
figure
surf(diffuse_3D)
axis([1 num_points_x 1 num_points_y 0 11e-6])

total_gain_3D = reshape(gain_total_total, sqrt(num_receiver), sqrt(num_receiver))';
figure
surf(total_gain_3D)
axis([1 num_points_x 1 num_points_y 0 11e-6])


%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% L O C A L  F U N C T I O N S
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [link_gain, distance, costheta_transmitter, costheta_receiver] = gain(coord_tx, coord_rx, norm_tx, norm_rx, m, area)
costheta_transmitter = dot(norm_tx, (coord_rx-coord_tx)/norm(coord_rx-coord_tx));
costheta_receiver = dot(norm_rx,-1*(coord_rx-coord_tx)/norm(coord_rx-coord_tx));
costheta_transmitter(isnan(costheta_transmitter)) = 0;
costheta_receiver(isnan(costheta_receiver)) = 0;

distance = norm(coord_rx - coord_tx);
link_gain = (m+1) * area * (costheta_transmitter^m) * (costheta_receiver) / (2*pi*distance^2);
end
