#pragma once

#include "cebnf.hpp"

class PrintSyntaxTree {
public:

	static std::string print(const std::unique_ptr<cebnf::SyntaxNode>& root) {
		std::string res = root->getString() + std::string(" :: ") + std::to_string(root->getTokenID()) + std::string("\n");

		for (int i = 0; i < root->children.size(); ++i) {
			res += print_impl(root->children[i], 1);
		}

		return res;
	}

private:

	static std::string print_impl(const std::unique_ptr<cebnf::SyntaxNode>& node, unsigned int depth) {
		std::string res;
		for (int i = 0; i < depth; ++i) res += "|   ";

		res += node->getString() + std::string(" :: ") + std::to_string(node->getTokenID()) + std::string("\n");

		for (int i = 0; i < node->children.size(); ++i) {
			res += print_impl(node->children[i], depth + 1);
		}

		return res;
	}


};


