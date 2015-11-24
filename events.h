#ifndef EVENT_LIST_H
#define EVENT_LIST_H
//=============================================================================
// Определение кодов событий
#define EVENT_NULL         		254
// сообщения от пульта RC5
#define EVENT_RC5_POWER       	13
#define EVENT_RC5_MUTE       	14
#define EVENT_RC5_KEY0       	1
#define EVENT_RC5_KEY1      	2
#define EVENT_RC5_KEY2       	3
#define EVENT_RC5_KEY3       	4
#define EVENT_RC5_KEY4       	5
#define EVENT_RC5_KEY5       	6
#define EVENT_RC5_KEY6       	7
#define EVENT_RC5_KEY7       	8
#define EVENT_RC5_KEY8       	9
#define EVENT_RC5_KEY9       	10
#define EVENT_RC5_BANK       	11
#define EVENT_RC5_PP     	  	15
#define EVENT_RC5_MENU       	49
#define EVENT_RC5_PPP       	35
#define EVENT_RC5_UP       	    33
#define EVENT_RC5_DOWN       	34
#define EVENT_RC5_LEFT       	18
#define EVENT_RC5_RIGHT      	17
#define EVENT_RC5_SET       	60
#define EVENT_RC5_LEFT1      	22
#define EVENT_RC5_RIGHT1     	23
#define EVENT_RC5_TV  	     	32
#define EVENT_RC5_AV  	     	57
#define EVENT_RC5_SLEEP	     	39
#define EVENT_RC5_TIME 	     	43
#define EVENT_RC5_STEREO     	37
#define EVENT_RC5_I_II 	     	36
#define EVENT_RC5_FORVARD     	31
#define EVENT_RC5_LIGHT	     	47
#define EVENT_RC5_S  	     	56
#define EVENT_RC5_P  	     	55
#define EVENT_RC5_F  	     	51
#define EVENT_RC5_I  	     	53
#define EVENT_RC5_FP  	     	45
#define EVENT_RC5_FM  	     	44
#define EVENT_RC5_M  	     	42
#define EVENT_RC5_LINE 	     	61
// сообщения от кнопок
#define EVENT_KEY_SET       	100
#define EVENT_KEY_SET_LONG  	101
#define EVENT_KEY_SET_HOLD  	102
#define EVENT_KEY_SET_DOUBLE  	103
#define EVENT_KEY_LEFT      	104
#define EVENT_KEY_RIGHT     	105
// DS18x20 evwnts
#define EVENT_SENSOR_PRESSURE	110
#define EVENT_SENSOR_TEMP		111
#define EVENT_SENSOR_HIMUDATE	112
// Секундный таймер
#define EVENT_TIMER_SECOND		253
// Прочие события
#define EVENT_SHOW_FREQ			150
#define EVENT_SHOW_VOLUME		151
#define EVENT_STOP_SHOW_VOLUME	152
#define EVENT_SHOW_CHANEL		153

#define EVENT_SAVE_CHANEL		160
#define EVENT_SAVE_VOLUME		161
#define EVENT_SET_STATE_OPTION	200
//=============================================================================
#endif // EVENT_LIST_H
