function client(port)
%   provides a menu for accessing PIC32 motor control functions
%
%   client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf(['a: Read current sensor (ADC counts)       b: Read current sensor (mA)\n' ...
             'c: Read encoder (counts)                  d: Read encoder (deg)\n' ...
             'e: Reset encoder                          f: Set PWM (-100 to 100)\n'...
             'g: Set current gains                      h: Get current gains\n'...
             'i: set position gains                     j: Get position gains\n'...
             'k: Test current gains                     l: Go to angle (deg)\n'...
             'm: Load step trajectory                   n; Load cubic trajectory \n'...
             'o: Execute trajectory                     p: Unpower the motor\n'...
             'q: Quit                                   r: Get mode\n']);
    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
    
    % take the appropriate action
    switch selection
        case 'a' %read current sensor (ADC counts)
            icounts=fscanf(mySerial, '%d');
            fprintf('The motor current is %d ADC counts.\n',icounts)
        case 'b' %read current sensor (mA)
            ima=fscanf(mySerial,'%f');
            fprintf('The motor current is %f mA.\n',ima)
        case 'c' %read encoder (counts)
            counts=fscanf(mySerial, '%d');
            fprintf('The motor angle is %d counts.\n',counts)
        case 'd'          %read encoder               
            deg=fscanf(mySerial, '%d');
            fprintf('The motor angle is %d degree(s).\n',deg)
        case 'e'                        
            rdeg=fscanf(mySerial, '%d');
            fprintf('Current motor angle has been reset to %d degree(s).\n',rdeg)           
        case 'f'
            pwm= input('What PWM value would you like [-100 to 100]?\n');
            fprintf(mySerial, '%d\n',pwm);
        case 'g'
            i_kp=input('Enter your desired Kp current gain [2]: \n');
            fprintf(mySerial,'%f\n',i_kp);
            i_ki=input('Enter your desired Ki current gain [0.07]: \n');
            fprintf(mySerial,'%f\n',i_ki);
            fprintf('Sending Kp= %f and Ki=%f to the current controller.\n\n',i_kp,i_ki);
        case 'h'
            i_gains=fscanf(mySerial,'%f %f');
            fprintf('The current controller is using Kp=%f and Ki=%f. \n\n',i_gains(1),i_gains(2));
        case 'i'
            p_kp=input('Enter your desired Kp position gain [60]: \n');
            fprintf(mySerial,'%f\n',p_kp);
            p_ki=input('Enter your desired Ki position gain [0]: \n');
            fprintf(mySerial,'%f\n',p_ki);
            p_kd=input('Enter your desired Kd position gain [190]: \n');
            fprintf(mySerial,'%f\n',p_kd);
            fprintf('Sending Kp= %f, Ki=%f, and Kd=%f to the position controller.\n\n',p_kp,p_ki,p_kd);
        case 'j'
            p_gains=fscanf(mySerial,'%f %f %f');
            fprintf('The position controller is using Kp=%f , Ki=%f, Kd=%f. \n\n',p_gains(1),p_gains(2),p_gains(3));
        case 'k'
            read_plot_matrix(mySerial);
        case 'l'
            goal_deg=input('Enter the desired motor angle in degrees: ');
            fprintf(mySerial,'%d\n',goal_deg);
        case 'm'
            test=[0,0;1,180;3,-90;4,0;5,0];
            traj=test;
            %traj=input('Enter step trajectory:');
            pos=genRef(traj,'step');
            fprintf(mySerial,'%d\n',length(pos));
            fprintf('Sending %d data...\n',length(pos));
            for i=1:length(pos)
                fprintf(mySerial,'%d\n',pos(i));
            end
            msg=fscanf(mySerial,'%d');
            fprintf('PIC32: Recieved %d data\n\n',msg);
        case 'n'
            test=[0,0;1,180;3,-90;4,0;5,0];
            traj=test;
            %traj=input('Enter step trajectory:');
            pos=genRef(traj,'cubic');
            fprintf(mySerial,'%d\n',length(pos));
            fprintf('Sending %d data...\n',length(pos));
            for i=1:length(pos)
                fprintf(mySerial,'%f\n',pos(i)); 
            end
            msg=fscanf(mySerial,'%d');
            fprintf('PIC32: Recieved %d data.\n\n',msg);
        case 'o'
            read_p_plot_matrix(mySerial);
        case 'p'
            fprintf('Unpower the motor.\n');
        case 'q'
            has_quit = true;             % exit client
        case 'r'
            modes=fscanf(mySerial,'%d');
            if modes==0
                mode='IDLE'; 
            elseif modes==1
                mode='PWM';
            elseif modes==2
                mode='ITEST';
            elseif modes==3
                mode='HOLD';
            elseif modes==4
                mode='TRACK';
            end
            fprintf(' The PIC32 controller mode is currently %s.\n',mode);
        case 'x'
            n= input('Enter first number:');
            fprintf(mySerial, '%d\n',n);
            m= input('Enter second number:');
            fprintf(mySerial, '%d\n',m);
            r = fscanf(mySerial,'%d');
            fprintf('Read: %d\n',r); 
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end
