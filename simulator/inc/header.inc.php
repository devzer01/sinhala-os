<!DOCTYPE html>
<html lang="si">
<head>
    <meta charset="latin1">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Title</title>
    <script src="assets/font.js"></script>
    <link rel="stylesheet" href="kbd.css" />
    <style>
        @font-face {
            font-family: sk8;
            src: url('WARNA.ttf');
        }

        .keyboard {
            width: 450px;
            float: left;
        }
        .keyboard kbd {
            font-family: sk8;
            float: left;
            width: 20px; height: 20px;
        }

        .dxkx {
            font-family: sk8;
        }

        .divpixel {
            width: 24px;
            height: 24px;
            background-color: white;
            float: left;
            border: gray 1px dotted;
            color: bisque;
            font-size: 1px;
        }
        .pxdig {
            height: 24px;
            border: gray 1px dotted;
            float: left;
            padding: 0 0;
            text-align: center;
        }
        .xred {
            border-bottom: red 1px solid !important;
        }

        .middlex {
            border-right: 2px solid black !important;
        }
        sup {
            vertical-align: super;
            font-size: smaller;
        }
        .keycap .border { stroke: black; stroke-width: 2; }
        .keycap .inner.border { stroke: rgba(0,0,0,.1); }
    </style>
    <link href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
</head>
<body style="width: 100%">
<a href="map.php">Map</a>