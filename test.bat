del out\*.*
del DEBUG.TXT 

dosview -s OUT\out.bmp images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.gif images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.pcx images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.tga images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.qoi images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.web images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.jpg images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.tif images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.jp2 images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.pnm images\640.jpg >>DEBUG.TXT
dosview -s OUT\out.ras images\640.jpg >>DEBUG.TXT
dosview -s OUT\outras.png out.ras >>DEBUG.TXT

dosview -s OUT\out2.bmp images\640.png >>DEBUG.TXT
dosview -s OUT\out2.gif images\640.png >>DEBUG.TXT
dosview -s OUT\out2.pcx images\640.png >>DEBUG.TXT
dosview -s OUT\out2.tga images\640.png >>DEBUG.TXT
dosview -s OUT\out2.qoi images\640.png >>DEBUG.TXT
dosview -s OUT\out2.web images\640.png >>DEBUG.TXT
dosview -s OUT\out2.jpg images\640.png >>DEBUG.TXT
dosview -s OUT\out2.tif images\640.png >>DEBUG.TXT
dosview -s OUT\out2.jp2 images\640.png >>DEBUG.TXT
dosview -s OUT\out2.pnm images\640.png >>DEBUG.TXT
dosview -s OUT\out2.ras images\640.png >>DEBUG.TXT
dosview -s OUT\outras2.png out2.ras >>DEBUG.TXT

dosview -s OUT\low.jpg -q 10 images\IMG_1940.png >>DEBUG.TXT
dosview -s OUT\low.web -q 10 images\IMG_1940.png >>DEBUG.TXT
dosview -s OUT\low.jp2 -q 10 images\IMG_1940.png >>DEBUG.TXT

dosview -s OUT\dblout.jpg -f 2.0 images\640.qoi >>DEBUG.TXT
dosview -s OUT\dblout.gif -f 2.0 images\640.qoi >>DEBUG.TXT
dosview -s OUT\hlfout.png -f 0.5 images\640.jpg >>DEBUG.TXT

dosview -h >>DEBUG.TXT
dosview -l >>DEBUG.TXT
dosview -w 642 images\640.jpg >>DEBUG.TXT
dosview -s OUT\fail.XXX images\640.jpg >>DEBUG.TXT
dosview -s OUT\fail.jpg -q 0 images\640.png >>DEBUG.TXT
dosview -s OUT\fail.jpg -q 101 images\640.png >>DEBUG.TXT

type DEBUG.TXT
