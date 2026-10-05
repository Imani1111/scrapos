#include <string.h>

void kstrcpy(char* dest, const char* src){
	while (*src != '\0'){
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
}

void* kmemcpy(void* dest, const void* src, uint32_t num){
	if (num == 0) return NULL;
	
	void* ret = dest;
	asm volatile (
			"rep movsb"
			: "+D" (dest), "+S" (src), "+c" (num)
			:
			: "memory"
		     );
	return ret;
}

int kstrcmp(uint8_t* str1, uint8_t* str2){
	while(*str1 && (*str1 == *str2)){
		str1++;
		str2++;
	}
	return *(const uint8_t*)str1 - *(const uint8_t*)str2;
}

void* kmemset(void* dest, uint8_t val, int count)
{
	uint8_t* ret = (uint8_t*)dest;
	for (int i = 0; i < count; i++){
		*ret = val;
		ret++;
	}
	return ret;
}

uint32_t kstrlen(char* s)
{
	uint32_t c = 0;
	while(*s++){
		c++;	
	}
	return c;
}

void itoa(int no, char* buffer)
{
	int i = 0;
	int is_negative = 0;

	if (no == 0){
		buffer[i++] = '0';
		buffer[i] = '\0';
		return;
	}
	if (no < 0){
		is_negative = 1;
		no = -no;
	}
	while (no != 0){
		int t = no % 10;
		buffer[i++] = t + '0';
		no = no / 10;
	}
	if (is_negative) buffer[i++] = '-';
	int start = 0;
	int end = i - 1;
	while(start < end){
		char s = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = s;
		start++;
		end--;
	}
	buffer[i] = '\0';
}

int kstrtok(char delim, char* str, char** token_arr, int token_count)
{
	char p[256] = {0};
	kmemcpy(p, str, kstrlen(str));
	int tc = 0;
	char* ptr = p;
	if (*ptr == '\0') return -1;
	while (*ptr == delim) ptr++;
	while (*ptr && (tc < token_count)){
		token_arr[tc++] = ptr;
		while (*ptr && *ptr != delim){
			ptr++;
		}
		if (*ptr == delim){
			*ptr = '\0';
			ptr++;
		}
	}
	return tc;
}

void* kmemmove(void* dest, void* src, int size){
	if (size == 0 || dest == src) return dest;
	uint8_t* d = (uint8_t*)dest;
	uint8_t* s = (uint8_t*)src;

	if (d < s){
		kmemcpy(d, s, size);
	}
	else{
		for (int i = size - 1; i > 0; i--){
			d[i] = s[i];
		}
	}
	return dest;
}
