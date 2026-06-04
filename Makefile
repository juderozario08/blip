all:
	./scripts.sh
file:
	./scripts.sh file
dev:
	./scripts.sh dev
test:
	./scripts.sh test
clean:
	rm -rf .cache build
	./scripts.sh
