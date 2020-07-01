#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>
#include <assert.h>

/*C++ Extended Backus-Naur Form (beta version)

Exchange:
"text" -> Term("text")
concatenation: comma( , ) -> hyphen( - ) (not exception symbol)
optional: [ ebnf ] -> t[ ebnf ] with CEBNF_OperatorTools t;
repetition: { ebnf } -> t( ebnf ) with CEBNF_OperatorTools t;
or  { ebnf } -> t({ ebnf }) with CEBNF_OperatorTools t;

<---note--->
Cebnf does not implemnet exception symbol.

If string including repetition and last string are same, for example ( { A | B | C }, A ), cebnf cannot parse it. Please create helper.

Alternation( | ) should be NAND(at least one is false). When both are true, then cebnf adopts one with a longer acquisition string.
For example, if the string "abcd..." matches both ("abc" ... and "ab" ...), then cebnf adopts "abc".

<---error--->
cebnf::CEBNF<TokenID> f;
t({ f })
std::initializer_list cannot use left reference value.
sorry, must add plus operator (wrapping).
t({ +f })

*/
namespace cebnf {

	class SyntaxNode {
		friend class SyntaxNode;
	public:
		SyntaxNode(std::size_t token_id, std::string s) : _token_id(token_id), _s(s) {}
		//SyntaxNode(SyntaxNode const& m) : _token_id(m._token_id), _s(m._s), children(m.children) {}
		SyntaxNode(SyntaxNode&& m) noexcept : _token_id(m._token_id), _s(m._s), children(std::move(m.children)) {
		}

		std::vector<std::unique_ptr<SyntaxNode>> children;

		std::size_t getTokenID() { return _token_id; }
		std::string getString() { return _s; }

		void mergeChildren(std::vector<std::unique_ptr<SyntaxNode>>&& new_children) {
			children.reserve(children.size() + new_children.size());
			std::move(new_children.begin(), new_children.end(), std::back_inserter(children));
		}

	private:
		std::size_t _token_id;
		std::string _s;
	};

	using SyntaxTree = std::pair<std::vector<std::unique_ptr<SyntaxNode>>, std::size_t>;

	struct Tree {
		static std::pair<std::vector<std::unique_ptr<SyntaxNode>>, std::size_t> Create(std::size_t token_id, std::string s, std::size_t len) {
			std::vector<std::unique_ptr<SyntaxNode>> vec;
			vec.emplace_back(new SyntaxNode(token_id, s));
			return std::move(std::pair<std::vector<std::unique_ptr<SyntaxNode>>, std::size_t>(std::move(vec), len));
		}
		static std::pair<std::vector<std::unique_ptr<SyntaxNode>>, std::size_t> Create() {
			std::vector<std::unique_ptr<SyntaxNode>> vec;
			return std::move(std::pair<std::vector<std::unique_ptr<SyntaxNode>>, std::size_t>(std::move(vec), 0));
		}
	};



	enum cebnf_type {
		base_begin = 10000000,
		terminus,
		epsilon,
		base_end
	};

	struct CEBNFBase {
		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) = 0;
	};

	template<typename type>
	using CEBNFBase_is_base_of_t = typename std::enable_if < std::is_base_of < CEBNFBase, typename std::remove_reference<type>::type>::value > ::type*;


	/*value class*/

	template<std::size_t TokenID>
	class CEBNF;

	struct Term;


	/*operator class*/

	template<class CEBNF_A, class CEBNF_B>
	struct To;
	template<class CEBNF_A, class CEBNF_B>
	struct Or;
	template<class CEBNF_A>
	struct May;
	template<class CEBNF_A>
	struct Rep;
	template<class CEBNF_A>
	struct Mul;
	template<class CEBNF_A>
	struct Wrap;

	struct tagValue {};
	struct tagOperator{};


	/*operator*/

	template<class CEBNF_L, class CEBNF_R, CEBNFBase_is_base_of_t<CEBNF_L> = nullptr, CEBNFBase_is_base_of_t<CEBNF_R> = nullptr>
	To<CEBNF_L, CEBNF_R> operator-(CEBNF_L&& l, CEBNF_R&& r) noexcept {
		return To<CEBNF_L, CEBNF_R>(std::forward<CEBNF_L>(l), std::forward<CEBNF_R>(r));
	}

	template<class CEBNF_L, class CEBNF_R, CEBNFBase_is_base_of_t<CEBNF_L> = nullptr, CEBNFBase_is_base_of_t<CEBNF_R> = nullptr>
	Or<CEBNF_L, CEBNF_R> operator|(CEBNF_L&& l, CEBNF_R&& r) noexcept {
		return Or<CEBNF_L, CEBNF_R>(std::forward<CEBNF_L>(l), std::forward<CEBNF_R>(r));
	}

	template<class CEBNF_L, class CEBNF_R, typename std::enable_if<std::is_integral<CEBNF_L>::value>::type* = nullptr, CEBNFBase_is_base_of_t<CEBNF_R> = nullptr>
	Mul<CEBNF_R> operator*(CEBNF_L l, CEBNF_R&& r) noexcept {
		return Mul<CEBNF_R>(l, std::forward<CEBNF_R>(r));
	}

	template<class CEBNF_R, CEBNFBase_is_base_of_t<CEBNF_R> = nullptr>
	Wrap<CEBNF_R> operator+(CEBNF_R&& r) noexcept {
		return Wrap<CEBNF_R>(std::forward<CEBNF_R>(r));
	}

	template<std::size_t TokenID>
	class CEBNF : public CEBNFBase, tagValue {
		friend class CEBNF<TokenID>;
	public:
		using tagCEBNF = void;
		CEBNF(){}

		~CEBNF() {
			if (_func != nullptr) {
				delete _func;
				//std::cout << "delete _func" << std::endl;
			}
		}

		//EBNF(EBNF const& eq) : _func(eq._func) {}

		/*TypeErasureClass*/
		//template<class Type, CEBNFBase_is_base_of_t<Type> = nullptr>
		//struct CEBNF_holder : public CEBNFBase {

		//	CEBNF_holder(Type h) : held(std::forward<Type>(h)) {}

		//	virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) {
		//		return held.parse_impl(s, begin);
		//	}

		//	Type held;
		//};

		//template<typename CEBNF_EQ, CEBNFBase_is_base_of_t<CEBNF_EQ> = nullptr>
		//CEBNF(CEBNF_EQ const& eq) {
		//	_func = std::shared_ptr<CEBNFBase>(new CEBNF_holder<CEBNF_EQ>(eq));
		//}

		//template<typename CEBNF_EQ, CEBNFBase_is_base_of_t<CEBNF_EQ> = nullptr>
		//CEBNF& operator=(CEBNF_EQ const& eq) {
		//	_func = std::shared_ptr<CEBNFBase>(new CEBNF_holder<CEBNF_EQ>(eq));
		//	return *this;
		//}

		template<typename CEBNF_EQ, CEBNFBase_is_base_of_t<CEBNF_EQ> = nullptr>
		CEBNF(CEBNF_EQ const& eq) {
			_func = new CEBNF_EQ(eq);
		}

		template<typename CEBNF_EQ, CEBNFBase_is_base_of_t<CEBNF_EQ> = nullptr>
		CEBNF& operator=(CEBNF_EQ const& eq) {
			_func = new CEBNF_EQ(eq);
			return *this;
		}



		std::unique_ptr<SyntaxNode> parse(const std::string& s) {
			SyntaxTree&& res_tree = parse_impl(s, 0);
			if (res_tree.first.size() == 0 || res_tree.second != s.length()) {
				return nullptr;
			}
			else {
				return std::move(res_tree.first[0]);
			}
		}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& children = _func->parse_impl(s, begin);

			if (children.first.size() == 0) {
				return Tree::Create();
			}

			auto&& I = Tree::Create(TokenID, s.substr(begin, children.second), children.second);
			I.first[0]->mergeChildren(std::move(children.first));
			return std::move(I);
		}

	private:
		CEBNFBase* _func = nullptr;
	};

	struct Term : public CEBNFBase, tagValue {

		std::size_t TokenID = cebnf_type::terminus;
		std::string _s = "";

		Term(const std::string& s) : _s(s)            {}
		Term(std::string&&      s) : _s(std::move(s)) {}
		Term(const char*        s) : _s(s)            {}

		Term(const std::string& s, std::size_t token_id) : _s(s)           , TokenID(token_id) {}
		Term(std::string&&      s, std::size_t token_id) : _s(std::move(s)), TokenID(token_id) {}
		Term(const char*        s, std::size_t token_id) : _s(s)           , TokenID(token_id) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			if (s.size() - begin >= _s.size() && std::equal(_s.begin(), _s.end(), s.begin() + begin)) {
				return Tree::Create(TokenID, _s, _s.length());
			}
			else {
				return Tree::Create();
			}
		}

	};

	template<class CEBNF_A, class CEBNF_B>
	struct To: public CEBNFBase, tagOperator {

		CEBNF_A _a;
		CEBNF_B _b;

		To(CEBNF_A a, CEBNF_B b) : _a(std::forward<CEBNF_A>(a))    , _b(std::forward<CEBNF_B>(b))     {}
		To(To const& eq)         : _a(eq._a)                       , _b(eq._b)                        {}
		To(To&& eq     )         : _a(std::forward<CEBNF_A>(eq._a)), _b(std::forward<CEBNF_B>(eq._b)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& childrenA = _a.parse_impl(s, begin);

			if (childrenA.first.size() == 0) {
				return Tree::Create();
			}

			//std::string sub_s = s.substr(childrenA.second);
			SyntaxTree&& childrenB = _b.parse_impl(s, begin + childrenA.second);

			if (childrenB.first.size() == 0) {
				return Tree::Create();
			}

			if (childrenA.first[0]->getTokenID() == cebnf_type::epsilon) {
				return std::move(childrenB);
			}
			else if (childrenB.first[0]->getTokenID() == cebnf_type::epsilon) {
				return std::move(childrenA);
			}
			else {
				childrenA.first.reserve(childrenA.first.size() + childrenB.first.size());
				std::move(childrenB.first.begin(), childrenB.first.end(), std::back_inserter(childrenA.first));
				//childrenB.first->clear();
				childrenA.second += childrenB.second;
				return std::move(childrenA);
			}

		}

	};

	template<class CEBNF_A, class CEBNF_B>
	struct Or : public CEBNFBase, tagOperator {

		CEBNF_A _a;
		CEBNF_B _b;

		Or(CEBNF_A a, CEBNF_B b) : _a(std::forward<CEBNF_A>(a))    , _b(std::forward<CEBNF_B>(b))     {}
		Or(Or const& eq)         : _a(eq._a)                       , _b(eq._b)                        {}
		Or(Or&& eq)              : _a(std::forward<CEBNF_A>(eq._a)), _b(std::forward<CEBNF_B>(eq._b)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& childrenA = _a.parse_impl(s, begin);
			SyntaxTree&& childrenB = _b.parse_impl(s, begin);

			if (childrenA.first.size() == 0 && childrenB.first.size() == 0) {
				return Tree::Create();
			}

			if (childrenA.first.size() > 0){
				if (childrenB.first.size() > 0) {

					if (childrenA.second >= childrenB.second) {
						return std::move(childrenA);
					}
					else {
						return std::move(childrenB);
					}
				}

				return std::move(childrenA);
			}
			else {
				return std::move(childrenB);
			}

		}

	};

	template<class CEBNF_A>
	struct May : public CEBNFBase, tagOperator {

		CEBNF_A _a;

		May(CEBNF_A a)     : _a(std::forward<CEBNF_A>(a))     {}
		May(May const& eq) : _a(eq._a)                        {}
		May(May&& eq)      : _a(std::forward<CEBNF_A>(eq._a)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& children = _a.parse_impl(s, begin);

			if (children.first.size() == 0) {
				return Tree::Create(cebnf_type::epsilon, "", 0);
			}
			else {
				return std::move(children);
			}
		}
	};

	template<class CEBNF_A>
	struct Rep : public CEBNFBase, tagOperator {

		CEBNF_A _a;

		Rep(CEBNF_A a)     : _a(std::forward<CEBNF_A>(a))     {}
		Rep(Rep const& eq) : _a(eq._a)                        {}
		Rep(Rep&& eq)      : _a(std::forward<CEBNF_A>(eq._a)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& children = _a.parse_impl(s, begin);

			if (children.first.size() == 0) {
				return Tree::Create(cebnf_type::epsilon, "", 0);
			}

			while (true) {
				//std::string sub_s = s.substr(children.second);
				SyntaxTree&& childrenX = _a.parse_impl(s, begin + children.second);

				if (childrenX.first.size() == 0) {
					return std::move(children);
				}
				children.first.reserve(children.first.size() + childrenX.first.size());
				std::move(childrenX.first.begin(), childrenX.first.end(), std::back_inserter(children.first));
				//childrenX.first->clear();
				children.second += childrenX.second;
			}
		}
	};

	template<class CEBNF_A>
	struct Mul : public CEBNFBase, tagOperator {

		unsigned int _n;
		CEBNF_A _a;

		Mul(unsigned int n, CEBNF_A a) : _n(n)    , _a(std::forward<CEBNF_A>(a))     {}
		Mul(Mul const& eq)             : _n(eq._n), _a(eq._a)                        {}
		Mul(Mul&& eq)                  : _n(eq._n), _a(std::forward<CEBNF_A>(eq._a)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			SyntaxTree&& children = _a.parse_impl(s, begin);

			if (children.first.size() == 0) {
				if (_n == 0) {
					return Tree::Create(cebnf_type::epsilon, "", 0);
				}
				else {
					return Tree::Create();
				}
			}

			for (unsigned int i = 1; i < _n; ++i) {
				SyntaxTree&& childrenX = _a.parse_impl(s, begin + children.second);

				if (childrenX.first.size() == 0) {
					return Tree::Create();
				}

				children.first.reserve(children.first.size() + childrenX.first.size());
				std::move(childrenX.first.begin(), childrenX.first.end(), std::back_inserter(children.first));
				//childrenX.first->clear();
				children.second += childrenX.second;
			}

			return std::move(children);
		}
	};

	template<class CEBNF_A>
	struct Wrap : public CEBNFBase, tagOperator {

		CEBNF_A _a;

		Wrap(CEBNF_A a)      : _a(std::forward<CEBNF_A>(a))     {}
		Wrap(Wrap const& eq) : _a(eq._a)                        {}
		Wrap(Wrap&& eq)      : _a(std::forward<CEBNF_A>(eq._a)) {}


		virtual SyntaxTree parse_impl(const std::string& s, const std::size_t begin) override {
			return _a.parse_impl(s, begin);
		}
	};

	template<class T>
	class has_tagCEBNF {
		template<class U> static constexpr std::true_type  check(typename U::tagCEBNF*);
		template<class U> static constexpr std::false_type check(                  ...);
	public:
		static constexpr bool value = decltype(check<T>(nullptr))::value;
	};

	template<class T>
	using has_tagCEBNF_t     = typename std::enable_if< has_tagCEBNF<T>::value>::type*;
	template<class T>
	using has_not_tagCEBNF_t = typename std::enable_if<!has_tagCEBNF<T>::value>::type*;

	struct CEBNF_OperatorTools {

		template<class CEBNF_C, CEBNFBase_is_base_of_t<CEBNF_C> = nullptr>
		May<CEBNF_C> operator[](CEBNF_C&& c) {
			return May<CEBNF_C>(std::forward<CEBNF_C>(c));
		}

		template<class CEBNF_C, CEBNFBase_is_base_of_t<CEBNF_C> = nullptr>
		Rep<CEBNF_C> operator()(CEBNF_C&& c) {
			return Rep<CEBNF_C>(std::forward<CEBNF_C>(c));
		}

		template<class CEBNF_C, CEBNFBase_is_base_of_t<CEBNF_C> = nullptr>
		Rep<CEBNF_C> operator()(std::initializer_list<CEBNF_C> c) {
			return Rep<CEBNF_C>(std::move(*(c.begin())));
		}

	};
	


}
