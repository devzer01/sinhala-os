<?php

require_once "../inc/mapread.php";

$langs_array = [
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

$ucs = new ucd();
$test = new tmap();
list($rows, $cols) = $test->load();

if (!isset($argv[1])) {
    printf("please provide language identifier\n");
    exit(0);
}

$LANG = $argv[1];
$debug = false;
if (isset($argv[2]) && $argv[2] === 'D') $debug = true;



$header = $test->get_header();

$cons = $test->get_lang_map($LANG);

$range = $test->get_range($LANG);
$ucd = new ucd();
$dbucs = [];
$data[0] = [$range[0], $range[1]];
for ($i = $range[0]; $i < $range[1]; $i++) {
    $point = $test->num2point($i);
    if ($ucd->codepoint($point) == 1) {
        $meta = $ucd->item($point);
        $index = array_search($i, $cons);
        if ($index === FALSE) {
            $mark = "*";
            if ($meta[2][0] == "M") $mark = "'";
            else if ($meta[2][0] == "P") $mark = ".";
            else if ($meta[2][0] == "N") $mark = "#";
            else if ($meta[2][0] == "S") $mark = "%";
          if ($debug === true)  printf("%s %s - %d - (%s) %s => %d * * * \n", $mark, $meta[0], hexdec($meta[0]), $meta[2], $meta[1], $index);
        } else {
          if ($debug === true) printf("+ %s - %d - %s => %s , %s, %s \n", $meta[0], hexdec($meta[0]), $meta[1], $header[0][$index], $header[1][$index], dechex($cons[$index]));
        }
        $ofs = hexdec($meta[0]);
        $oidx = ($ofs - $range[0]);
        $data[$oidx + 1] = [$header[1][$index], dechex($oidx + ($range[0] & 0xff)), 0x00, 0x00];
    } else {
        if (array_search($i, $cons) !== FALSE) {
            printf("- unknown mapping found at %d %s \n", $i, dechex($i));
        }
    }
}

function frmt($inp) {
    return str_pad(strtoupper(dechex($inp)), 2, "0", STR_PAD_LEFT);
}
$start_r1 = ($data[0][0] >> 8);
$start_r2 = ($data[0][0] & 0x00ff);

$end_r1 = ($data[0][1] >> 8);
$end_r2 = ($data[0][1] & 0x00ff);
printf("const unsigned char si_conf[516] = { %s, %s, %s, %s,\n", frmt($start_r1), frmt($start_r2), frmt($end_r1), frmt($end_r2));
for ($i = 1; $i < count($data); $i++) {
	$nomark = ",";
	if ($i === count($data) - 1) $nomark = "";
    if (isset($data[$i])) {
        $v = $data[$i];
        printf("0x%s, 0x%s, 0x00, 0x00%s ", frmt($v[0]), frmt($v[1]), $nomark);
    } else {
        printf("0x00, 0x00, 0x00, 0x00%s ", $nomark);
    }
    if ($i % 3 == 0) {
        printf("\n");
    }
}
printf("};\n");

