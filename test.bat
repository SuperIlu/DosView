del out.*
del lala.txt

dosview -s out.bmp images\640.jpg >>lala.txt
dosview -s out.pcx images\640.jpg >>lala.txt
dosview -s out.tga images\640.jpg >>lala.txt
dosview -s out.qoi images\640.jpg >>lala.txt
dosview -s out.web images\640.jpg >>lala.txt
dosview -s out.jpg images\640.png >>lala.txt
dosview -s out.tif images\640.png >>lala.txt

dosview -s low.jpg -q 10 images\IMG_1940.png >>lala.txt
dosview -s low.web -q 10 images\IMG_1940.png >>lala.txt

dosview -h >>lala.txt
dosview -l >>lala.txt
dosview -w 642 images\640.jpg >>lala.txt
dosview -s fail.XXX images\640.jpg >>lala.txt
dosview -s fail.jpg -q 0 images\640.png >>lala.txt
dosview -s fail.jpg -q 101 images\640.png >>lala.txt

type lala.txt
