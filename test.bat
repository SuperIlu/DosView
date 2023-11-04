del out.*

dosview -s out.bmp images\640.jpg
dosview -s out.pcx images\640.jpg
dosview -s out.tga images\640.jpg
dosview -s out.qoi images\640.jpg
dosview -s out.web images\640.jpg
dosview -s out.jpg images\640.png
dosview -s out.tif images\640.png

dosview -s low.jpg -q 10 images\IMG_1940.png
dosview -s low.web -q 10 images\IMG_1940.png

dosview -h
dosview -l
dosview -w 642 images\640.jpg
dosview -s fail.XXX images\640.jpg
dosview -s fail.jpg -q 0 images\640.png
dosview -s fail.jpg -q 101 images\640.png
