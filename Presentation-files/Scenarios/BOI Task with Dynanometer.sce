# -------------------------- Header Parameters --------------------------

scenario = "BOI Task"; 

#write_codes = EXPARAM( "Send ERP Codes" );

max_y = 100;
active_buttons = 2;
button_codes= 1, 2;
response_matching = simple_matching;

no_logfile = false;
response_logging = log_active;
write_codes = true;
response_port_output = false;

	
stimulus_properties = 
	type, string,
	subject_id, string,
	block_name, string,
	trial_number, string,
	ITI, string,
	event_cond_1, string,
	event_cond_2, string,
	file_name, string,
	dyno_response, string; 
event_code_delimiter = ";";	

# ------------------------------- SDL Part ------------------------------
begin;

#Set default picture so can make it white later in PCL
picture {} default;

# ------------------------------- Sound Stimuli -------------------------
wavefile {filename = ""; preload = false;} the_sound_stimuli_wavefile; #an sound object which we will use in each trial. Filename will be iteratively updated in each trial. 
sound { wavefile the_sound_stimuli_wavefile; } the_sound_stimuli_object;

# ------------------------------- Trials --------------------------------
trial {
	trial_duration = forever;
	trial_type = specific_response;
	terminator_button = 1; # only SPACE
	
	stimulus_event{
		picture { 
			text { 
				caption = " "; 
				preload = false;
				font_size = 4.5;
			} instruct_text; 
			x = 0; 
			y = 0;
		}; 
		code = "Instruction";
		port_code = 32;
		code_width = indefinite_port_code; #Tried to use stimulus_event::INDEFINITE_PORT_CODE but throws error.
	} instruct_event;
} instruct_trial; # for practice instr and test instr, manually replace caption

trial {
	clear_active_stimuli = false;
	trial_duration = stimuli_length;
	monitor_sounds = true; # sounds are not terminated if a trial ends
	all_responses = false; # response during stimuli presentation is disabled
	
	stimulus_event {
		sound { 
			wavefile { 
				filename = ""; 
				preload = false; 
			}; 
		} sound_anchors;
		code = "Stimuli";
		code_width = indefinite_port_code;	
	} sound_event;	
	
	stimulus_event { 
		picture {
			text { 
				caption = "";
				preload = false; 
				font_size = 6;
			} anchor_text;
			x = 0;
			y = 0;
		} label_anchors;
	} sound_label_event;
} sound_trial; 

trial {
	stimulus_event {
			picture{};
	code = "ITI";
	port_code = 8;
	code_width = indefinite_port_code; #Tried to use stimulus_event::INDEFINITE_PORT_CODE but throws error. 
	} ITI_event;
}ITI_trial;

trial { 
   trial_duration = forever;
	trial_type = specific_response;
	terminator_button = 1, 2; # press SPACE to end break, R to repeat
	
	stimulus_event {
		picture {
			text {
				caption = "";
				preload = false; 
				font_size = 6;
				} end_block_text;
			x = 0; 
			y = 0;
		} end_block_pic;
		code = "End_Block";
		port_code = 32;
		code_width = indefinite_port_code; #Tried to use stimulus_event::INDEFINITE_PORT_CODE but throws error.
	} end_block_event;
} end_block_trial;

trial {	
	trial_duration = 2000;
	stimulus_event {
		picture { 
			text { 
				caption = "";
				preload = false;
				font_size = 6; 
			} fb_text; 
			x = 0; 
			y = 0; 
		}fb_pic;
		code = "Feedback";
		port_code = 32;
		code_width = indefinite_port_code; #Tried to use stimulus_event::INDEFINITE_PORT_CODE but throws error.
	} feedback_event;
} fb_trial;

trial{
	trial_type = specific_response;
	terminator_button = 1;
	trial_duration = forever;
	stimulus_event {
		picture {
			text {
				caption = " ";
				preload = false;
				font_size = 30;
			}caption_1;
			x = 0; y =0;
			text {
				caption = "(Press SPACE to move on)";
				font_size = 5;
			}caption_2;
			x = 0; y = -85;
		} break_pic;
		code = "Break";
		port_code = 16;
		code_width = indefinite_port_code; #Tried to use stimulus_event::INDEFINITE_PORT_CODE but throws error. 
	}break_event;
} break_trial;

picture {
   text { 
	caption = "Enter participant number:"; 
	font_size = 5	;
};
   x = 0; y = 0;
   text { caption = " "; font_size = 2;} parti_num_text;
   x = 0; y = -20;
} parti_num;

# ----------------------------- PCL Program -----------------------------
begin_pcl;

if (input_port_manager.port_count() < 1) then
   exit( "Something wrong with Dynomometer or SensorDAQ, Presentation cannot detect port device." )
end;

set_random_seed( int(logfile.subject()) );					#use participant number as seed

input_port in_port = input_port_manager.get_port( 1 ); 
output_port out_port = output_port_manager.get_port( 1 );

#Constant Integers
int ONE_SEC = 1000;
int TWO_SEC = 2000;
int THREE_SEC = 3000;


int outport_D0 = 1;
int outport_D1 = 2;
int outport_D2 = 4;
int outport_D3 = 8;
int outport_D4 = 16;
int outport_D5 = 32;

int reset_outport = 0;
int Low_BOI_Port = outport_D0; 						#1bit signal
int High_BOI_Port = outport_D1;						#2bit signal
int Abstract_BOI_Port = outport_D2;					#4bit signal
int ITI_Port = outport_D3;								#8bit signal
int Break_Port = outport_D4
int Others_Port = outport_D5


output_file outputFile = new output_file;
outputFile.open( logfile.subject() + ".txt", false ); # don't overwrite
outputFile.print( "Subject Number,Block,Trial Number,Condition1,Condition2,Stimulus,Dynomometer Response,Trial Duration\n" );


# ----------------------------- Import Sound File Paths-------------------------------------

include_once "..\\PCLs\\Import_Sounds.pcl";

# ----------------------------- Functions ----------------------------------------------

include_once "..\\PCLs\\Import_Functions.pcl";

# ----------------------------- Import Message File ------------------------------------
language_file lang = new language_file;
lang.load( stimulus_directory + "English.xml" );

# ----------------------------- Study Block --------------------------------------------
play_instruct_trial( lang.get_text( "Instructions" ) );
include_once "..\\PCLs\\Study_Block.pcl";
anchor_text.set_caption( "" ); #reset caption since in practice phase we don't show caption along with stimuli
play_break_trial();
play_ITI_trial( ONE_SEC ); #Just a short blank screen between blocks

# ----------------------------- PRACTICE Block ------------------------------------------

#play_instruct_trial( lang.get_text( "Response Prompt Caption" ) );
#include_once "..\\PCLs\\Practice_Block.pcl";
#play_break_trial();
#play_instruct_trial( lang.get_text("Practice Complete Caption" ) );
#play_ITI_trial( ONE_SEC ); #Just a short blank screen between blocks


# ----------------------------- TEST Block -----------------------------------------------

play_instruct_trial( lang.get_text( "Response Prompt Caption" ) );
include_once "..\\PCLs\\Test_Block.pcl";
play_ITI_trial( ONE_SEC ); #Just a short blank screen between blocks

# ----------------------------- END ------------------------------------------------------
play_instruct_trial( lang.get_text( "Completion Screen Caption" ) );
outputFile.close();