<?php

require_once "mapread.php";

$lang_array = [
	['0xF','0B00','0B7F','Odia'],
	['0x1','980','9FF','Assamese'],
	['0x3','980','9FF','Bengali'],
	['0x6','900','97F','Devanagari'],
	['0x7','A80','AFF','Gujarati'],
	['0x8','A00','A7F','Gurmukhi'],
	['0x16','0F00','0FFF','Tibetan'],
	['0x4','11000','1107F','Brahmi'],
	['0x14','0C00','0C7F','Telugu'],
	['0xA','0C80','0CFF','Kannada'],
	['0x10','0D80','0DFF','Sinhala'],
	['0xE','0D00','0D7F','Malayalam'],
	['0x13','0B80','0BFF','Tamil'],
	['0x5','1000','104F','Burmese'],
	['0xB','1780','17FF','Khmer'],
	['0x15','E00','0E7F','Thai'],
	['0xC','E80','EFF','Lao'],
	['0x2','1B00','1B7F','Balinese'],
	['0x9','A980','A9DF','Javanese'],
	['0x11','1B80','1BFF','Sundanese'],
	['0XD','1A00','1A7F','Lontara']
];


foreach($lang_array as $v) {

    $runner = new Runner($v[0], strtolower($v[3]));
    $runner->build_map(false);
    $runner->format_header(strtolower($v[3]) . ".h");
}
