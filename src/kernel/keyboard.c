#include <keyboard.h>
#include <idt.h>
#include <screen.h>

KeyboardKeyCollector key_collector = {0};

uint8_t shift_pressed = 0;
uint8_t capslock_active = 0;
uint8_t is_extended = 0;

extern void isr33();

const char keyboard_keys[] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',0,'*', 0, ' '
};

const char keyboard_keys_shifted[] = {
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static inline uint8_t decode_scancode(uint8_t scancode){
	if (scancode >= (sizeof(keyboard_keys))) return 0;
	uint8_t c = keyboard_keys[scancode];
	uint8_t use_uppercase = shift_pressed ^ capslock_active;
	if (c >= 'a' && c <= 'z'){
		return use_uppercase ? keyboard_keys_shifted[scancode] : c; 
	}
	return shift_pressed ? keyboard_keys_shifted[scancode] : c;
}

static inline void write_key(KeyboardKeyCollector* collector, uint8_t key){
	uint8_t next_write_index = (collector->writing_index + 1) % MAX_KEYS;
	if (next_write_index == collector->reading_index){
		return;
	}
	collector->key_buffer[collector->writing_index] = key;
	collector->writing_index = next_write_index;	
}

char read_key(){
	uint8_t next_read_index = (key_collector.reading_index + 1) % MAX_KEYS;
	if (key_collector.reading_index == key_collector.writing_index){
		return 0;
	}
	char key = key_collector.key_buffer[key_collector.reading_index];
	key_collector.reading_index = next_read_index;
	return key;
}

void HandleKeyboardInterrupt()
{
	uint8_t scancode = read_byte_from_port(KEYBOARD_DATA_PORT);
	uint8_t key_identifier = scancode & 0x7F;
	uint8_t key_event = scancode & 0x80;

	if (scancode == 0xE0){
		is_extended = 1;
		PIC_sendEOI(33);
		return;
	}
	
	if (is_extended){
		switch(scancode){
			case WINDOWS_BUTTON: {
				write_key(&key_collector, WINDOWS_BUTTON);
				break;
			}case LEFT_CTRL: {
				write_key(&key_collector, LEFT_CTRL);
				break;
			}		
			default:
				   break;
		}
		PIC_sendEOI(33);
	}

	if (key_event == 0){
		switch(key_identifier){
			case LEFT_SHIFT:
			case RIGHT_SHIFT: {
				shift_pressed = 1;
				break;
			}
			case CAPS_LOCK: {
				capslock_active = !capslock_active;
				break;
			}
			case UP: {
				move_cursor(0, -1);
				break;
			}
			case DOWN: {
				move_cursor(0, 1);
				break;
			}
			case LEFT: {
				move_cursor(-1, 0);
				break;
			}
			case RIGHT: {
				move_cursor(1, 0);
				break;
			}	    
			default: {
				uint8_t key = decode_scancode(key_identifier);
				write_key(&key_collector, key);
			}
		}
	}
	else{
		if (key_identifier == LEFT_SHIFT || key_identifier == RIGHT_SHIFT){
			shift_pressed = 0;
		}
	}

	PIC_sendEOI(33);
}
