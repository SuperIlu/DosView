del out.*
del DEBUG.TXT

dosview -s out.bmp images\640.jpg >>DEBUG.TXT
dosview -s out.pcx images\640.jpg >>DEBUG.TXT
dosview -s out.tga images\640.jpg >>DEBUG.TXT
dosview -s out.qoi images\640.jpg >>DEBUG.TXT
dosview -s out.web images\640.jpg >>DEBUG.TXT
dosview -s out.jpg images\640.png >>DEBUG.TXT
dosview -s out.tif images\640.png >>DEBUG.TXT
dosview -s out.jp2 images\640.png >>DEBUG.TXT
dosview -s out.pnm images\640.png >>DEBUG.TXT
dosview -s out.ras images\640.png >>DEBUG.TXT
dosview -s outras.png out.ras >>DEBUG.TXT

dosview -s low.jpg -q 10 images\IMG_1940.png >>DEBUG.TXT
dosview -s low.web -q 10 images\IMG_1940.png >>DEBUG.TXT
dosview -s low.jp2 -q 10 images\IMG_1940.png >>DEBUG.TXT

dosview -s dblout.jpg -f 2.0 images\640.qoi >>DEBUG.TXT
dosview -s hlfout.png -f 0.5 images\640.jpg >>DEBUG.TXT

dosview -h >>DEBUG.TXT
dosview -l >>DEBUG.TXT
dosview -w 642 images\640.jpg >>DEBUG.TXT
dosview -s fail.XXX images\640.jpg >>DEBUG.TXT
dosview -s fail.jpg -q 0 images\640.png >>DEBUG.TXT
dosview -s fail.jpg -q 101 images\640.png >>DEBUG.TXT

type DEBUG.TXT
