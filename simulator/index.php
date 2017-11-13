<?php include_once "inc/header.inc.php"; ?>
<div style="width: 50%; float: left">
    <canvas id="canvas" width="640" height="480"></canvas>
    <br />
    Resolution:
    <input type="radio" class="res" name="res" val="1" data-width="640" data-height="480">640x480
    <input type="radio" class="res" name="res" val="2" data-width="800" data-height="600">800x600
    <input type="radio" class="res" name="res" val="3" data-width="1024" data-height="768">1024x768

        <input type="text" size="2" id="leftx">
        <button id="xclear">Clear</button>
        <button id="swap">Swap</button>
        <button id="xcopy">Copy</button>
        <input type="text" size="2" id="rightx">




    <div style="width: 100%">
        <div style="width: 600px; float: left;">
            <?php for ($i = 128; $i < 256; $i++) {
                $zx = $i-128;
                ?>
                <div style="float: left; width: 12%" data-xpos="<?php echo $zx;?>" class="dxpos">
                    <canvas style='float: left' id="pcanvas<?php echo $i;?>" width="100" height="100"></canvas>
                    <div style='float: left' class='dxkx' data-code="<?php echo $i;?>"><?php $x = chr($i); echo $x . "($zx)";?></div>
                </div>
            <?php } ?>
        </div>
    </div>
</div>
<div style="width: 50%; float: left">
    <div>
        <div id="gcode" style="width: 150px; height:418px; float: left; font-size: 1px; border: black 1px double; ">
            <?php for ($i = 0; $i < 16; $i++) { ?>
                <div style="width: 100%">
                    <div style="width: 30%;" class='pxdig' id="hex<?php echo $i;?>">0x00</div>
                    <div style="width: 65%;" class='pxdig' id="bin<?php echo $i;?>">0000 0000</div>
                </div>
            <?php } ?>
        </div>
        <div id="grid" style="width: 209px; height: 418px; border: black 1px double; float: left;">
            <?php for ($i = 0; $i < 16*8; $i++) {
                $extraclass = "";
                if (($i >= 8*3 && $i < 8*4) || ($i >= 8*11 && $i < 8*12)) {
                    $extraclass = "xred";
                }
                if (($i % 8) == 7) {
                    //$extraclass .= " middlex ";
                }
                ?>
                <div id="pxl<?php echo $i;?>" class="divpixel <?php echo $extraclass; ?>" data-bin="0" data-pixel="<?php echo $i; ?>" aria-label="<sup><?php $z = ($i % 16); echo $z; ?></sup>"></div>
            <?php } ?>
        </div>
        <div id="grid2" style="width: 209px; height: 418px; border: black 1px double; float: left;">
            <?php for ($i = (16*8); $i < (16*8) + 16*8; $i++) {
                $extraclass = "";
                if (($i >= 16*3 && $i < 16*4) || ($i >= 16*11 && $i < 16*12)) {
                    $extraclass = "xred";
                }
                if (($i % 8) == 7) {
                    //$extraclass .= " middlex ";
                }
                ?>
                <div id="pxr<?php echo $i;?>" class="divpixel <?php echo $extraclass; ?>" data-bin="0" aria-label="<sup><?php $z = ($i % 16); echo $z; ?></sup>"></div>
            <?php } ?>
        </div>
        <div style="float: left; width: 30px">&nbsp;;</div>
        <div class="rxh" style="float: left">
            <div style="float: left; width: 100%">
            <canvas id="preview" width="16" height="16" style="border: 1px solid black; float: left; margin-top: 15px; margin-left: 10px;"></canvas>
            <hr style="clear: both"/>
                <input type="radio" name='side' class="left" selected>Left
                <input type="radio" name='side' class="right">Right
            </div>
            <div id="currentchar"></div>
            <div class="keyboard">
                <?php for ($i = 128; $i < 256; $i++) { ?>
                    <kbd class='xkx' data-code="<?php echo $i;?>"><?php $x = chr($i); echo $x;?></kbd>
                <?php } ?>
            </div>
            <div>
                <button id="left"><</button>
                <button id="right">></button>
                <button id="up">/\</button>
                <button id="down">\/</button>
                <button id="save">Save</button>
            </div>
        </div>
    </div>
</div>

<?php include_once "inc/footer.inc.php"; ?>