<?php

define("C_LANG", "A");
define("C_START", "B");
define("C_END", "C");
define("R_CLASS", 1);
define("R_CLCODE", 2);
define("R_VCLASS", 26);
define("R_VLCODE", 27);

class ucd {

    public $sheet = [];

    protected $index = [
        'punch' => []
    ];

    protected $langs = [
        ['0x1', '980', '9FF'],
        ['0x3', '980', '9FF'],
        ['0x6', '900', '97F'],
        ['0x7', 'A80', 'AFF'],
        ['0xF', '0B00', '0B7F'],
        ['0x8', 'A00', 'A7F'],
        ['0x16', '0F00', '0FFF'],
        ['0x12', 'A800', 'A82F'],
        ['0x4', '11000', '1107F'],
        ['0x14', '0C00', '0C7F'],
        ['0xA', '0C80', '0CFF'],
        ['0x10', '0D80', '0DFF'],
        ['0xE', '0D00', '0D7F'],
        ['0x13', '0B80', '0BFF'],
        ['0x5', '1000', '104F'],
        ['0xB', '1780', '17FF'],
        ['0x15', '0E00', '0E7F'],
        ['0xC', 'E80', 'EFF'],
        ['0x2', '1B00', '1B7F'],
        ['0x9', 'A980', 'A9DF'],
        ['0x11', '1B80', '1BFF'],
        ['0xD', '1A00', '1A1F']
    ];


    public function __construct()
    {
        $fp = fopen("UnicodeData.txt", "r");
        while (FALSE != ($row = fgetcsv($fp, 0, ";"))) {
            $idx = strtoupper(dechex(hexdec($row[0])));
            $this->sheet[$idx] = $row;
            switch ($row[2]) {
                case "Po":
                    $this->index['punch'][$idx] = $row;
                    break;
            }
        }
        fclose($fp);
    }

    public function get_punch($langs = [])
    {
        $retval = [];
        foreach ($this->index['punch'] as $p => $r) {
            foreach ($this->langs as $l) {
                if (!empty($langs) && !in_array($l[0], $langs)) continue;
                if (hexdec($p) >= hexdec($l[1]) && hexdec($p) <= hexdec($l[2])) {
                    $retval[] = $r;
                }
            }
        }

        return $retval;
    }

    public function codepoint($codepoint)
    {
        return isset($this->sheet[$codepoint]);
    }

    public function item($codepoint)
    {
        return isset($this->sheet[$codepoint]) ? $this->sheet[$codepoint] : -1;
    }
}

class tmap {

    protected $fp = null;

    protected $sheet = [];

    protected $rows = 0;

    protected $cols = 0;

    protected $datastart = "D";

    protected $datacols = [];

    protected $class_header = [];

    protected $byte_map = [];

    protected $range_start = 0;

    protected $range_end = 0;

    public function __construct()
    {

    }



    public function load()
    {
        $fp = fopen("map.csv", "r");
        while (FALSE != ($row = fgetcsv($fp))) {
            $this->sheet[] = $row;
        }
        fclose($fp);
        $this->rows = count($this->sheet);
        $this->cols = count($this->sheet[0]);

        $this->class_header = $this->cl_load(R_CLASS, R_VCLASS);
        $this->byte_map = $this->cl_load(R_CLCODE, R_VLCODE);
        return [$this->rows, $this->cols];
    }

    public function langs()
    {
        $start = 3; $end = 24;
        $lrec = [];
        for ($i = $start; $i <= $end; $i++) {
            $lrec[] = [$this->cell($i, C_LANG), $this->cell($i, C_START), $this->cell($i, C_END)];
        }
        return $lrec;
    }

    public function r_load($row, $offset = 0)
    {
        $cl = [];
        for ($i = $this->col2num($this->datastart), $idx = $offset; $i < ($this->col2num($this->datastart) + $this->cols - 3); $i++, $idx++) {
            $col = $this->num2col($i);
            if ($row == 38) {
                printf("search %d %s", $idx, $col);
            }
            $val = $this->cell($row, $col);
            if ($row == 38) {
                printf(" val %s \n", $val);
            }
            if ($val != "") {
                $cl[$idx] = $val;
            }
        }
        return $cl;
    }

    public function cl_load($row, $row2)
    {
        $c = $this->r_load($row);
        $v = $this->r_load($row2, count($c));
        $m =  array_merge($c, $v);
        ksort($m);
        return $m;
    }

    public function get_header()
    {
        return [$this->class_header, $this->byte_map];
    }

    public function num2col($num)
    {
        if ($num < 27) {
            return chr($num + 65);
        } else {
            return chr(64 + ($num / 26)) . chr(65 + ($num % 26));
        }
    }

    public function col2num($col)
    {
        $c_col = ord($col[0]) - 65;
        if (isset($col[1])) {
            $c_col = ord($col[1]) - 65 + 26;
        }
        return $c_col;
    }

    public function find_lang_record($lang)
    {
        $cs = -1; $vr = -1;
        for ($i = R_CLCODE + 1; $i < R_VCLASS; $i++) {
            if ($this->cell($i, C_LANG) == $lang) {
                $cs = $i;
                break;
            }
        }
        for ($i = R_VLCODE + 1; $i < R_VLCODE + 22; $i++) {
            if ($this->cell($i, C_LANG) == $lang) {
                $vr = $i;
                break;
            }
        }

        return [$cs, $vr];
    }

    public function get_map_with_meta($lang)
    {
        $lang = $this->get_lang_map($lang);

        $retval = [];
        foreach($this->class_header as $idx => $cls) {
            $record = [];
            if ($lang[$idx] == 0) {
            } else {
                $record = [$cls, $this->byte_map[$idx], $lang[$idx], $idx, "&#x" . dechex($lang[$idx]) . ";"];
                $idx2 = strtoupper(dechex($record[2]));
                $retval[$idx2] = $record;
            }


        }
        return $retval;
    }

    public function get_range($lang) {
        $this->seek_lang($lang);
        return [$this->range_start, $this->range_end];
    }

    protected function seek_lang($lang) {
        list($cs, $vw) = $this->find_lang_record($lang);
        if ($cs == -1 || $vw == -1) {
            printf("language not found\n");
            exit;
        }
        $this->range_start = hexdec($this->cell($cs, C_START));
        $this->range_end = hexdec($this->cell($cs,C_END));

        return [$cs, $vw];
    }

    public function get_lang_map($lang) {
        list($cs, $vw) = $this->seek_lang($lang);
        return $this->cl_load($cs, $vw);
    }

    public function num2point($num)
    {
        if (hexdec(dechex($num)) !== $num) {
            throw new Exception("none decimal value " . $num);
        }
        return strtoupper(dechex($num));
    }

    public function cell($row, $col)
    {
        $c_col = $this->col2num($col);
        //debug_print_backtrace();
        return isset($this->sheet[--$row][$c_col]) ? $this->sheet[$row][$c_col] : debug_print_backtrace();
    }
}

