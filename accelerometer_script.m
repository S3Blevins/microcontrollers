input = serial('COM5', 'BaudRate', 9600);
input.ReadAsyncMode = 'continuous';

fopen(input);

midpoint = 128;
g_per_bit = 6/256;
gravity = 9.8;

%% set up plot with -30m/s^2 to 30m/s^2
figure
x_line = animatedline('Color', 'r');
y_line = animatedline('Color', 'b');
z_line = animatedline('Color', 'g');
ax = gca;
ax.YGrid = 'on';
ax.YLim = [-30 30];

stop = false;
startTime = datetime('now');
while ~stop
    x = 0;
    y = 0;
    z = 0;
    try
        string = strtrim(fscanf(input));
        string_split = split(string, ",");

        % x, y, and z axis as values (0-255)
        x = str2num(string_split{1});
        y = str2num(string_split{2});
        z = str2num(string_split{3});
    catch e
        fprintf("Index out of range, try again.");
        string_split = ["0","0","0"];
    end

    % convert axis 8-bit to g
    x_gravity = (x - midpoint) * g_per_bit;
    y_gravity = (y - midpoint) * g_per_bit;
    z_gravity = (z - midpoint) * g_per_bit;
    
    % convert g to m/s^2
    x_acceleration = x_gravity * gravity;
    y_acceleration = y_gravity * gravity;
    z_acceleration = z_gravity * gravity;
    
    % Get current time
    t =  datetime('now') - startTime;
    % Add points to animation
    
    try
        fprintf("\nx = %d\n", x_acceleration);
        fprintf("y = %d\n", y_acceleration);
        fprintf("z = %d\n", z_acceleration);
        
        addpoints(x_line,datenum(t),x_acceleration);
        addpoints(y_line,datenum(t),y_acceleration);
        addpoints(z_line,datenum(t),z_acceleration);
        % Update axes
        ax.XLim = datenum([t-seconds(5) t]);
        datetick('x','keeplimits')
        drawnow
        % Check stop condition
    catch e
        fprintf("Baud rate error. Continue");
    end

end
