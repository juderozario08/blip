all:
	./scripts.sh
dev:
	./scripts.sh dev
test:
	./scripts.sh test
clean:
	rm -rf .cache build
	./scripts.sh
