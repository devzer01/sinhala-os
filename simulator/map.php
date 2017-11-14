<?php include_once "inc/header.inc.php";

include_once "inc/map.php";
global $matrix8; global $sinhala;
?>

<h1>Vowels</h1>
<table class="table">
    <?php
    $group = "";
    for ($r = 0; $r < 6; $r++) {
        if ($group != $matrix8[$r][0][C_CLASS]) {
            $cols = array_fill(0, 8, "<td>%s</td>");
            array_walk($cols, function(&$v, $k) {
                $v = "" . sprintf($v, str_pad($k, 2, "0", STR_PAD_LEFT));
            });
            printf("<tr><th>%s</th>%s</tr>", $matrix8[$r][0][C_CLASS], implode(" ", $cols));
            $group = $matrix8[$r][0][C_CLASS];
        }
        printf("<tr>");
        if ($matrix8[$r][0][C_CODE] % 8 == 0) {
            printf("<td rowspan='2'>0x%s</td>", strtoupper(dechex($matrix8[$r][0][C_CODE])));
        }
        for ($c = 0; $c < 8; $c++) { ?>
            <th><?php  echo $matrix8[$r][$c][C_NAME] . " (" .$matrix8[$r][$c][C_ATTR] . ") ";  ?></th>
            <?php
        }
        printf("%s", "</tr>");
        printf("%s", "<tr>");
        for ($c = 0; $c < 8; $c++) {
            $sin = $matrix8[$r][$c]['si'];
            ?>
            <td><?php  printf("&#x%s", dechex($sin[1]));  ?></td>
            <?php
        }
        printf("%s", "</tr>");
    }
    ?>
</table>
<h1>Consonants</h1>
<table class="table">
    <?php
    $group = "";
    for ($r = 3; $r < 8; $r++) {
        if ($group != $matrix16[$r][0][C_CLASS]) {
            printf("<tr><th colspan='17'>%s</th></tr>", $matrix16[$r][0][C_CLASS]);
            $group = $matrix16[$r][0][C_CLASS];
        }
        printf("<tr>");
        if ($matrix16[$r][0][C_CODE] % 16 == 0) {
            printf("<td rowspan='2'>%s</td>", strtoupper(dechex($matrix16[$r][0][C_CODE])));
        }
        for ($c = 0; $c < 16; $c++) { ?>
            <th><?php  echo $matrix16[$r][$c][C_NAME] . " (" .$matrix16[$r][$c][C_ATTR] . ") ";  ?></th>
            <?php
        }
        printf("%s", "</tr>");
        printf("%s", "<tr>");
        for ($c = 0; $c < 16; $c++) {
            $sin = $matrix16[$r][$c]['si'];
            ?>
            <td><?php  printf("%s &#x%s", dechex($sin[0]), dechex($sin[1]));  ?></td>
            <?php
        }
        printf("%s", "</tr>");
    }
    ?>
</table>
<?php include_once "inc/footer.inc.php"; ?>
