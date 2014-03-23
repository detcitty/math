#ifndef STAN__GM__PARSER__FUNCTIONS__GRAMMAR_DEF__HPP__
#define STAN__GM__PARSER__FUNCTIONS__GRAMMAR_DEF__HPP__

#include <boost/spirit/include/qi.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <boost/spirit/include/version.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <stan/gm/ast.hpp>
#include <stan/gm/grammars/functions_grammar.hpp>
#include <stan/gm/grammars/statement_grammar.hpp>
#include <stan/gm/grammars/whitespace_grammar.hpp>


BOOST_FUSION_ADAPT_STRUCT(stan::gm::function_decl_def,
                          (stan::gm::bare_type, return_type_)
                          (std::string, name_) 
                          (std::vector<stan::gm::arg_decl>, arg_decls_)
                          (stan::gm::statement, body_) );

BOOST_FUSION_ADAPT_STRUCT(stan::gm::arg_decl,
                          (stan::gm::bare_type, arg_type_)
                          (std::string, name_)
                          (stan::gm::statement, body_) );

namespace stan {

  namespace gm {

    struct add_function_signature {
      template <typename T1>
      struct result { typedef void type; };
      void operator()(const function_decl_def& decl) const {
        expr_type result_type(decl.return_type_.base_type_,
                              decl.return_type_.num_dims_);
        std::vector<expr_type> arg_types;
        for (size_t i = 0; i < decl.arg_decls_.size(); ++i)
          arg_types.push_back(expr_type(decl.arg_decls_[i].arg_type_.base_type_,
                                        decl.arg_decls_[i].arg_type_.num_dims_));
        function_signatures::instance().add(decl.name_,result_type,arg_types);
      }
    };
    boost::phoenix::function<add_function_signature> add_function_signature_f;

    struct unscope_variables {
      template <typename T1, typename T2>
      struct result { typedef void type; };
      void operator()(function_decl_def& decl,
                      variable_map& vm) const {
        for (size_t i = 0; i < decl.arg_decls_.size(); ++i)
          vm.remove(decl.arg_decls_[i].name_);
      }
    };
    boost::phoenix::function<unscope_variables> unscope_variables_f;


    struct add_fun_var {
      template <typename T1, typename T2, typename T3, typename T4>
      struct result { typedef void type; };
      // each type derived from base_var_decl gets own instance
      void operator()(arg_decl& decl,
                      bool& pass,
                      variable_map& vm,
                      std::ostream& error_msgs) const {
        if (vm.exists(decl.name_)) {
          // variable already exists
          pass = false;
          error_msgs << "duplicate declaration of variable, name="
                     << decl.name_;

          error_msgs << "; attempt to redeclare as function argument";

          error_msgs << "; original declaration as ";
          print_var_origin(error_msgs,vm.get_origin(decl.name_));

          error_msgs << std::endl;
          return;
        }

        pass = true;
        vm.add(decl.name_,
               decl.base_variable_declaration(),
               function_argument_origin);
      }
    };
    boost::phoenix::function<add_fun_var> add_fun_var_f;

  template <typename Iterator>
  functions_grammar<Iterator>::functions_grammar(variable_map& var_map,
                                                 std::stringstream& error_msgs)
      : functions_grammar::base_type(functions_r),
        var_map_(var_map),
        error_msgs_(error_msgs),
        statement_g(var_map_,error_msgs_),
        bare_type_g(var_map_,error_msgs_)
  {
      using boost::spirit::qi::_1;
      using boost::spirit::qi::char_;
      using boost::spirit::qi::eps;
      using boost::spirit::qi::lexeme;
      using boost::spirit::qi::lit;
      using boost::spirit::qi::no_skip;
      using boost::spirit::qi::_pass;
      using boost::spirit::qi::_val;

      using boost::spirit::qi::labels::_a;
      using boost::spirit::qi::labels::_r1;
      using boost::spirit::qi::labels::_r2;

      using boost::spirit::qi::on_error;
      using boost::spirit::qi::fail;
      using boost::spirit::qi::rethrow;
      using namespace boost::spirit::qi::labels;

      functions_r.name("function declarations and definitions");
      functions_r 
        %= lit("functions") 
        >> lit('{')
        >> *function_r
        >> lit('}')
        ;
      
      function_r.name("function declaration or definition");
      function_r
        %= bare_type_g
        >> identifier_r
        >> lit('(')
        >> arg_decls_r
        >> lit(')')
        >> statement_g(false,local_origin)
        >> eps [ unscope_variables_f(_val,
                                     boost::phoenix::ref(var_map_)) ]
        >> eps [ add_function_signature_f(_val) ]
        ;
      
      arg_decls_r.name("function argument declaration sequence");
      arg_decls_r
        %= arg_decl_r % ',';

      arg_decl_r.name("function argument declaration");
      arg_decl_r 
        %= bare_type_g
        >> identifier_r
        >> eps[ add_fun_var_f(_val,_pass,
                              boost::phoenix::ref(var_map_),
                              boost::phoenix::ref(error_msgs_)) ]
        ;

      identifier_r.name("identifier");
      identifier_r
        %= (lexeme[char_("a-zA-Z") 
                   >> *char_("a-zA-Z0-9_.")]);

      
    }

  }
}
#endif

