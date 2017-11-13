var abugida = [
{ name: 'Ahom', unicode: '11700–1173F'},
'Brahmi – Sanskrit, Prakrit,',
'Balinese',
'Batak – Toba and other Batak languages', 
'Baybayin – Ilokano, Kapampangan, Pangasinan, Tagalog, Bikol languages, Visayan languages, and possibly other Philippine languages','Bengali-Assamese script – Bengali, Assamese, Meithei, Bishnupriya Manipuri, Kokborok, Khasi language, Bodo language.','Bhaiksuki',
'Buhid','Burmese – Burmese, Karen languages, Mon, and Shan','Cham','Chakma','Dehong – Dehong Dai','Devanagari – Hindi, Sanskrit, Marathi, Nepali, and many other languages of northern India','Dhives Akuru','Grantha – Sanskrit','Gujarati – Gujarāti, Kachchi','Gurmukhi script – Punjabi','Hanuno’o','Javanese','Kaithi','Kannada – Kannada, Tulu, Konkani, Kodava','Kawi','Khojki','Khotanese','Khudawadi','Khmer','Kolezhuthu – Tamil, Malayalam','Lao','Lepcha','Leke','Limbu','Lontara’ – Buginese, Makassar, and Mandar','Mahajani','Malayalam – Malayalam','Malayanma – Malayalam','Marchen – Zhang-Zhung','Meetei Mayek','Modi – Marathi','Multani – Saraiki','Nandinagari – Sanskrit','Newar – Nepal Bhasa, Sanskrit','New Tai Lue','Oriya','Pallava script – Tamil, Prakrit, Sanskrit','Phags-pa – Mongolian, Chinese, and other languages of the Yuan dynasty Mongol Empire','Ranjana – Nepal Bhasa, Sanskrit','Rejang','Sharada – Sanskrit','Siddham – Sanskrit','Sinhala','Sourashtra','Soyombo','Sundanese','Syloti Nagri – Sylheti','Tagbanwa – Languages of Palawan','Tai Le','Tai Dam','Tai Tham – Khün, and Northern Thai','Takri','Tamil','Telugu','Thai','Tibetan','Tigalari – Sanskrit','Tirhuta – Maithili','Tocharian','Vatteluttu – Tamil, Malayalam','Zanabazar Square','Zhang zhung scripts','Kharoṣṭhī (extinct), from the 3rd century BC','Geez (Ethiopic), from the 4th century AD',
'Canadian Aboriginal syllabics','Cree – Ojibwe syllabics','Inuktitut syllabics','Blackfoot syllabics','Carrier syllabics','Thaana','Pollard script','Pitman shorthand','Tengwar (fictional)','Ihathvé Sabethired (fictional)'
];

/**
 * Created by nayana on 10/26/17.
 */
var charattr = [
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000
];

var canvas = null;
var ctx = null;
var lcanvas = null;
var lctx = null;

var changeRes = function(height, width) {
    $("#canvas").width = width;
    $("#canvas").height = height;
    canvas.width = width;
    canvas.height = height;
    initCanvas();
    drawChars();
};

var initCanvas = function() {
    ctx.fillStyle = "black";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    lcanvas = document.getElementById("preview");
    lctx = lcanvas.getContext("2d");
    lcanvas.width = 16;
    lcanvas.height = 16;

    lctx.fillStyle = "black";
    lctx.fillRect(0, 0, lcanvas.width, lcanvas.height);
    lctx.strokeStyle = "#FF0000";

};

var drawChars = function () {
    drawpAll();
    var charwidth = 8;
    var charheight = 16;
    var charbytes = 16;
    var numchars = canvas.width / (charwidth + 1);

    var index = 0x80;
    var i = 0;
    var y = 0;
    ctx.strokeStyle = "#FF0000";
    var id = ctx.createImageData(1, 1); // only do this once per page
    var d = id.data;                        // only do this once per page
    d[0] = 255;
    d[1] = 255;
    d[2] = 255;
    d[3] = 255;
    var lasti = i;
    var currenty = y;
    for (var xx = 0; xx < bytex.length; xx++) {
        if (xx !== 0 && xx % 16 === 0) {
            if (canvas.width < (i + charwidth)) {
                currenty =+ 17;
                i = -9;
            }
            y = currenty;
            i = i + 2;
            lasti = i;
        } else {
            i = lasti;
        }
        var byte = bytex[xx];
        index = 0x80;
        while (index > 0) {
            //ctx.beginPath();
            if (index === (byte & index)) {
                ctx.putImageData(id, i, y);
            }
            index = index >> 1;
            i++;
        }
        y = y + 1;
    }
};

var drawpAll = function() {
    for (i = 0; i < 128; i++) {
        var code = i;
        var bmt = bytex.slice(16 * code, (16 * code) + 16);
        drawp(bmt, code + 128);
    }
};

var drawp = function(bitmask, pid) {
    var pcanvas = document.getElementById('pcanvas' + pid);
    var pctx = pcanvas.getContext("2d");
    pctx.fillStyle = "black";
    pcanvas.width = 10;
    pcanvas.height = 20;

    pctx.fillStyle = "black";
    pctx.fillRect(0, 0, pcanvas.width, pcanvas.height);
    pctx.strokeStyle = "#FF0000";

    var id = pctx.createImageData(1, 1); // only do this once per page
    var d = id.data;                        // only do this once per page
    d[0] = 255;
    d[1] = 255;
    d[2] = 255;
    d[3] = 255;

    var ix = 0;
    var bxi = 0;
    var li = 0;
    var ly = 0;
    for (ix = 0; ix < bitmask.length; ix++) {
        var byte = bitmask[ix];
        ly++;
        li = 0;
        var index = 0x80;
        while (index > 0) {
            if (index == (byte & index)) {
                d[0] = 255; d[1] = 255; d[2] = 255;
                pctx.putImageData(id, li, ly);
            } else {
                d[0] = 0; d[1] = 0; d[2] = 0;
                pctx.putImageData(id, li, ly);
            }
            bxi++;
            index = index >> 1;
            li++;
        }
    }

};

var drawpreview = function(bitmask, isRight) {


    var id = lctx.createImageData(1, 1); // only do this once per page
    var d = id.data;                        // only do this once per page
    d[0] = 255;
    d[1] = 255;
    d[2] = 255;
    d[3] = 255;

    var ix = 0;
    var bxi = 0;
    var li = 0;
    var ly = 0;
    for (ix = 0; ix < bitmask.length; ix++) {
        var byte = bitmask[ix];
        ly++;
        li = 0;
        var index = 0x80;
        while (index > 0) {
            if (index == (byte & index)) {
                d[0] = 255; d[1] = 255; d[2] = 255;
                if (isRight === true) {
                    lctx.putImageData(id, li + 9, ly);
                } else {
                    lctx.putImageData(id, li, ly);
                }
            } else {
                d[0] = 0; d[1] = 0; d[2] = 0;
                if (isRight === true) {
                    lctx.putImageData(id, li + 9, ly);
                } else {
                    lctx.putImageData(id, li, ly);
                }
            }
            bxi++;
            index = index >> 1;
            li++;
        }
    }
};

var encoding = function()
{
    var y = currentchar[x];
};

$(document).ready(function () {
    $(".res").change(function (e) {
        changeRes(e.target.dataset.height, e.target.dataset.width);
    });

    $(".divpixel").click(function (e) {
        var pixel = $(this).data('pixel');
        var bin =   $(this).data('bin');
        var irow = parseInt(pixel / 8);
        var ibit = pixel % 8;
        currentchar[irow] ^= 0x80 >> ibit;
        //|= (clear) &= (set)

       switch (bin) {
           case "0":
               $(this).css("background-color", "black");
               $(this).data('bin', "1");
               break;
           case "1":
               $(this).data('bin', "0");
               $(this).css("background-color", "white");
               break;
       }
        drawpreview(currentchar, false);
        setBinHex();
    });

    $("#up").click(function (e) {
       var last = currentchar.shift();
       currentchar.push(last);
       clearGrid();
       drawpreview(currentchar);
       drawGrid(currentchar);
       setBinHex();
    });
    $("#down").click(function (e) {
        var last = currentchar.pop();
        currentchar.unshift(last);
        clearGrid();
        drawpreview(currentchar);
        drawGrid(currentchar);
        setBinHex();
    });
    $("#left").click(function (e) {
        currentchar = currentchar.map(function (v) {
            var z = (v << 1);
            if (z > 255) {
                z = z & 0xff;
                z++;
            }
            return z;
        });
        clearGrid();
        drawpreview(currentchar);
        drawGrid(currentchar);
        setBinHex();
    });
    $("#right").click(function (e) {
        currentchar = currentchar.map(function (v) {
            var z = (v >> 1);
            if ((v & 0x01) === 0x01) {
                z = z + 0x80;
            }
            return z;
        });
        clearGrid();
        drawpreview(currentchar);
        drawGrid(currentchar);
        setBinHex();
    });
    $("#save").click(function (e) {
        if (currentchar !== null) {
            var index = 16 * currentCode;
            bytex.splice.apply(bytex, [index, 16].concat(currentchar));
        }
        jQuery.ajax({
            type: "POST",
            url: "save.php",
            data: bytex,
            processData: false,
            contentType: 'application/octet-stream',
            success: function (ex) {
                initCanvas();
                drawChars();
            },
            dataType: 'json'
        });
    });
    $(".dxpos").click(function (e) {
       var dpos = $(this).data('xpos');
        $("#leftx").val(dpos);
    });
    $("#swap").click(function (e) {
        var leftx = $("#leftx").val();
        var rightx = $("#rightx").val();
        var lbitmask = bytex.slice(16 * leftx, (16 * leftx) + 16);
        var rbitmask = bytex.slice(16 * rightx, (16 * rightx) + 16);
        bytex.splice.apply(bytex, [leftx * 16, 16].concat(rbitmask));
        bytex.splice.apply(bytex, [rightx * 16, 16].concat(lbitmask));
        initCanvas();
        drawChars();
    });

    $("#xcopy").click(function (e) {
        var leftx = $("#leftx").val();
        var rightx = $("#rightx").val();
        var lbitmask = bytex.slice(16 * leftx, (16 * leftx) + 16);
        //bytex.splice.apply(bytex, [leftx * 16, 16].concat(rbitmask));
        bytex.splice.apply(bytex, [rightx * 16, 16].concat(lbitmask));
        initCanvas();
        drawChars();
    });
    $("#xclear").click(function (e) {
        var leftx = $("#leftx").val();
        var lbitmask = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
        //bytex.splice.apply(bytex, [leftx * 16, 16].concat(rbitmask));
        bytex.splice.apply(bytex, [leftx * 16, 16].concat(lbitmask));
        initCanvas();
        drawChars();
    });


    canvas = document.getElementById("canvas");
    ctx = canvas.getContext("2d");
    canvas.width = 640;
    canvas.height = 480;
    initCanvas();
    drawChars();

    $(".xkx").click(function (e) {
        var code = $(this).data('code') - 128;
        var bitmask = bytex.slice(16 * code, (16 * code) + 16);

        if ($(".left:checked").length !== 0) {
            currentCode = code;
            currentchar = bitmask;
            clearGrid();
            drawpreview(bitmask);
            drawGrid(bitmask);
            setBinHex();
        } else {
            drawpreview(bitmask, true);
            drawGrid(bitmask, true);
        }

    });

});

var currentCode = 0;

var setBinHex = function() {
  currentchar.forEach(function (v, k) {
      var bin = v.toString(2).padStart(8,0).split("");
      bin.splice(4, 0, " ");
      $("#hex" + k).text("0x" + v.toString(16));
      $("#bin" + k).text(bin.join(""));
  });
  $("#currentchar").text(currentchar.join(""));
};

var clearGrid = function () {
  for (i=0; i < 16*8; i++) {
      $("#pxl" + i).css('background-color', "white");
  }
};

var drawGrid = function (bitmask, isRight) {
    var start = 0;
    var bxi = 0;
    var classNamePrefix = 'pxl';
    var extra = 0;
    if (isRight === true) {
        classNamePrefix = 'pxr';
        extra = 16*8;
    }
    var ix = 0;

    for (ix = start; ix < bitmask.length; ix++) {
        var byte = bitmask[ix];
        var index = 0x80;
        while (index > 0) {
            var xindex = (bxi + extra);
            if (index == (byte & index)) {
                $("#" + classNamePrefix + xindex).css('background-color', "black");
                $("#" + classNamePrefix + xindex).data('bin', "1");
            } else {
                $("#" + classNamePrefix + xindex).data('bin', "0");
            }
            bxi++;
            index = index >> 1;
        }
        //if (isRight) {
        //    bxi += 16;
       // }
    }
};

var currentchar = null;