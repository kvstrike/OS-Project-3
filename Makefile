all:
	gcc main1.c -o main1
	gcc main2.c -o main2
	gcc main3.c -o main3

clean:
	rm main1 main2 main3 out*