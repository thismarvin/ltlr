do {
	open src/config.h
	| lines
	| str replace '^//(#define SUPPORT_CUSTOM_FRAME_CONTROL)' '$1'
	| save config.h
}

do {
	rm src/config.h
	mv config.h src
}
