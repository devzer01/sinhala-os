<?php

require_once "mapread.php";


$ucs = new ucd();
var_dump($ucs->codepoint("D80"));
var_dump($ucs->codepoint("D82"));
//$punch = $ucs->get_punch();
$test = new tmap();
list($rows, $cols) = $test->load();
/*foreach ($punch as $meta) {
    printf("* %s - %d - %s => * * * \n", $meta[0], hexdec($meta[0]), $meta[1]);
}
exit;*/

if (!isset($argv[1])) {
    printf("please provide language identifier\n");
    exit(0);
}

$LANG = $argv[1];




/*list($v, $c) = $test->get_header();
$header = [];
foreach ($v as $k => $vx) {
    $header[] = [$v[$k], $c[$k]];
}
print_r($header);
exit;*/
//$test->verify("0x10");
//list ($map, $ucs) = $test->verify("0x10");
//var_dump($ucs);*/
//exit;*/

printf("col2num %s - %d \n", "Z", $test->col2num("Z"));
printf("col2num %s - %d \n", "AA", $test->col2num("AA"));
printf("col2num %s - %d \n", "AR", $test->col2num("AR"));
printf("col2num %s - %s \n", "43 -> AR", $test->num2col("43"));


$header = $test->get_header();

$cons = $test->get_lang_map($LANG);

//var_dump($cons); exit;

$result = array_search(3458, $cons);
printf("header records classification %s and byte code %s\n", $header[0][$result], $header[1][$result]);
printf("search for %d in map renders index %d and value @ is %d match %d\n", 3461, $result, $cons[$result], $cons[$result] == 3461);

$range = $test->get_range($LANG);
printf("range query returns %d (%s) - %d (%s) \n", $range[0], dechex($range[0]), $range[1], dechex($range[1]));

$col2num = $test->num2col(0);
if ($col2num == "A") {
    printf("col2num OK\n");
} else {
    printf("col2num fail %s\n", $col2num);
}

if ($rows === 46 && $cols === 45) {
    printf("rows and cols match\n");
} else {
    printf("rows and cols not match %d %d\n", $rows, $cols);
}

$cell_lookups = [
    [15, "BY", 2996],
    [21, "BU", 43435]
];

foreach ($cell_lookups as $lk) {
    $lookup = $test->cell($lk[0], $lk[1]);
    if ($lookup == $lk[2]) {
        printf("cell lookup match\n");
    } else {
        printf("cell %s lookup %s not match %s\n", $lk[0], $lk[1], $lookup);
    }
}

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
          //  printf("%s %s - %d - (%s) %s => %d * * * \n", $mark, $meta[0], hexdec($meta[0]), $meta[2], $meta[1], $index);
        } else {
          //  printf("+ %s - %d - %s => %s , %s, %s \n", $meta[0], hexdec($meta[0]), $meta[1], $header[0][$index], $header[1][$index], dechex($cons[$index]));
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
printf("const unsigned char si_conf[512] = { %s, %s, %s, %s,\n", frmt($start_r1), frmt($start_r2), frmt($end_r1), frmt($end_r2));
for ($i = 1; $i < count($data); $i++) {
    if (isset($data[$i])) {
        $v = $data[$i];
        printf("0x%s, 0x%s, 0x00, 0x00, ", frmt($v[0]), frmt($v[1]));
    } else {
        printf("0x00, 0x00, 0x00, 0x00, ");
    }
    if ($i % 3 == 2) {
        printf("\n");
    }
}
printf("};\n");

