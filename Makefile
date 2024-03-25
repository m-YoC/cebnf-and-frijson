.DEFAULT_GOAL := help


docker-up: ## Start all container.
	docker-compose up -d
docker-down: ## Down all container.
	docker-compose down
docker-build: ## Build all container.
	docker-compose build

git-up: ## git add all and commit. (this commnad do not push)
	@git add . && git commit -m "no comment (committed with git-up command of makefile)"
	@echo "\n  Please type 'git push' to upload at GitHub.\n"

help: ## Display this help screen.
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk -F ':.*?## ' '{printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

