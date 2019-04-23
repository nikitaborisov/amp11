all:
	python3 setup.py build
	cp build/lib*/* .
clean:
	rm -fr build *.so cuda_outer_product_sum.cpp
