#ifndef BIN_DOT_GRAPH_HEADER
#define BIN_DOT_GRAPH_HEADER

#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <string>

#include "rose.h"
#include "BinaryControlFlow.h"
#include "BinaryLoader.h"

using namespace std;

typedef rose::BinaryAnalysis::ControlFlow::Graph CFG;
typedef rose::BinaryAnalysis::FunctionCall::Graph CG;
typedef boost::graph_traits<CFG>::vertex_iterator CFGVIter;
typedef boost::graph_traits<CFG>::edge_iterator CFGEIter;
typedef boost::graph_traits<CG>::vertex_iterator CG_VIter;

class BinaryDotGenerator {
   private:
      bool includeInst;
      ofstream f;
      map<SgAsmBlock *, string> nameMap;
      string stringFormatter(string s) {
         stringstream ss;
         for(size_t i = 0; i < s.size(); i++)
             switch(s[i]) {
              case '&':     ss << "&amp;";      break;
              case '\"':    ss << "&quot;";     break;
              case '\'':    ss << "&apos;";     break;
              case '<':     ss << "&lt;";       break;
              case '>':     ss << "&gt;";       break;
              default:      ss << s[i];         break;
             }
         return ss.str();                    
      }
   public:
      BinaryDotGenerator(CFG &cfg, string programName, string filename, bool includeInst_ = true) : includeInst(includeInst_), f(filename.c_str()) {
         //Initialize
           if(!f.good())
              return;
           f << "/* Auto generated DOT graph." << endl
             << "  Compiler .dot->.png: \"dot -Tpng " << filename << " > " << filename << ".png\"" << endl
             << "  (The format was heavily insipred by Kalani Thielen's example at http://www.graphviz.org/content/psg)*/" << endl << endl
             << "digraph G {" << endl << "compound=true;" << endl
             << "ranksep=1.25;" << endl << "fontsize=30;" << endl
             << "labelloc=\"t\";" << "label=\"Project: '" << programName << "'\";" << endl
             << "bgcolor=white;" << endl
             << endl;
         //Perform and callgraph analysis...
           rose::BinaryAnalysis::FunctionCall cg_analyzer;
           /*struct ExcludeLeftovers: public BinaryAnalysis::FunctionCall::VertexFilter {
               bool operator()(BinaryAnalysis::FunctionCall *analyzer, SgAsmFunction *func) {
                  return func && 0==(func->get_reason() & SgAsmFunction::FUNC_LEFTOVERS);
               }
             } exclude_leftovers;
             cg_analyzer.set_vertex_filter(&exclude_leftovers);*/
           CG cg = cg_analyzer.build_cg_from_cfg<CG>(cfg);
         //Iterate over all blocks...
           map<SgAsmFunction *, size_t> bbIndex;
           for(pair<CFGVIter, CFGVIter> vP = vertices(cfg); vP.first != vP.second; ++vP.first) {
               SgAsmBlock *bb = get(boost::vertex_name, cfg, *vP.first);
               SgAsmFunction *func = bb->get_enclosing_function();

               if(bbIndex.find(func) == bbIndex.end())
                  bbIndex[func] = 0;
               bbIndex[func]++;

               (*this)(bb, (func == NULL ? "Unknown func" : func->get_name()), bbIndex.find(func)->second);
           }
         //Iterate over all functions...
           for(pair<CG_VIter, CG_VIter> vP = vertices(cg); vP.first != vP.second; ++vP.first) {
               SgAsmFunction *func = get(boost::vertex_name, cg, *vP.first);

               (*this)(func, func->get_name(), cfg);
           }
         //Add all outside edges...
           f << endl;
           for(pair<CFGEIter, CFGEIter> eP = edges(cfg); eP.first != eP.second; ++eP.first) {
               SgAsmBlock *src = get(boost::vertex_name, cfg, source(*eP.first, cfg)),
                          *dst = get(boost::vertex_name, cfg, target(*eP.first, cfg));
               if(src->get_enclosing_function() != dst->get_enclosing_function())
                  f << " " << nameMap.find(src)->second << " -> " << nameMap.find(dst)->second
                    << " [penwidth=1 fontsize=28 fontcolor=\"black\" label=\"\"];" << endl;
           }
           f << "}" << endl;
         //Finalize
           f.close();
      }

      void operator()(SgAsmBlock *bb, string name, int index) {
         if(bb == NULL)
            return;
         stringstream bbName;
         bbName << name << index;
         nameMap[bb] = bbName.str();
         f << " \"" << bbName.str() 
           << "\" [style=\"filled\" penwidth=1 fillcolor=\"white\" fontname=\"Courier New\" shape=\"Mrecord\" label="
           << "<"
           << "<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">"
           << "<tr>"
           << "<td bgcolor=\"black\" align=\"center\" colspan=\"2\">"
           << "<font color=\"white\">" << "BB #" << bb->get_id() << "</font>"
           << "</td>"
           << "</tr>";
         int Iindex = 0, Dindex = 0;
         if(includeInst)
            //iterator is for a std::vector
            for(SgAsmStatementPtrList::iterator it = bb->get_statementList().begin(); 
                it != bb->get_statementList().end(); ++it) {
                  stringstream ss;
                  switch((*it)->variantT()) {
                   case V_SgAsmBlock:
                   case V_SgAsmFunction:
                     ROSE_ASSERT(false);
                   case V_SgAsmInstruction:
                   case V_SgAsmArmInstruction:
                   case V_SgAsmPowerpcInstruction:
                   case V_SgAsmMipsInstruction:
                   case V_SgAsmX86Instruction:
                     {
                      string s = unparseInstructionWithAddress(static_cast<SgAsmInstruction *>(*it));
                      boost::replace_all(s, "<", "[");
                      boost::replace_all(s, ">", "]");
                      ss << "[I" << Iindex << "] " << s;
                      Iindex++;
                     } break;
                   case V_SgAsmStaticData:
                     ss << "[D" << Dindex << " (" << ")] "; //static_cast<SgAsmStaticData *>(*it)->get_size() <<
                     for(SgUnsignedCharList::iterator it2 = static_cast<SgAsmStaticData *>(*it)->get_raw_bytes().begin();
                         it2 != static_cast<SgAsmStaticData *>(*it)->get_raw_bytes().end();
                         ++it2)
                               ss << std::hex << *it2 << " ";
                     Dindex++;
                     break;
                   default:
                     ROSE_ASSERT(false && "Unhandled IR type");
                  }
                  f << "<tr><td align=\"left\" port=\"r4\">" << ss.str() << "</td></tr>";
            }
            f << "</table>> ];" << endl;            
      }

      void operator()(SgAsmFunction *func, string name, CFG &cfg) {
         if(func == NULL)
            return;
         string kind, cc;
         switch(func->get_function_kind()) {
          case SgAsmFunction::e_unknown:    kind = "Unknown";   break;
          case SgAsmFunction::e_standard:   kind = "Standard";  break;
          case SgAsmFunction::e_library:    kind = "Library";   break;
          case SgAsmFunction::e_imported:   kind = "Imported";  break;
          case SgAsmFunction::e_thunk:      kind = "Thunk";     break;
          case SgAsmFunction::e_last:       kind = "Last";      break;
         }
         /*switch(func->get_functionCallingConvention()) {
          case SgAsmFunction::e_unknown_call:    cc = "Unknown";     break;
          case SgAsmFunction::e_std_call:        cc = "Std";         break;
          case SgAsmFunction::e_fast_call:       cc = "Fast";        break;
          case SgAsmFunction::e_cdecl_call:      cc = "CDecl";       break;
          case SgAsmFunction::e_this_call:       cc = "This";        break;
          case SgAsmFunction::e_last_call:       cc = "Last";        break;
         }*/

         //replace all . in the name for underscore to avoid errors in dot/dotty.
         boost::replace_all(name, ".", "_");

         f << "subgraph cluster" << name << " {" << endl << "node [style=filled];" << endl
           << "color=blue;" << endl << "fontsize=20;" << endl
           << "label=\"Function '" << func->get_name() << "' (Kind=" << kind << ", CC=" << cc << ")\";" << endl
           << "labelloc=\"t\";" << endl << "overlap=false;" << endl
           << "rankdir=\"LR\";" << endl << endl;
         for(pair<CFGEIter, CFGEIter> eP = edges(cfg); eP.first != eP.second; ++eP.first) {
             SgAsmBlock *src = get(boost::vertex_name, cfg, source(*eP.first, cfg)),
                        *dst = get(boost::vertex_name, cfg, target(*eP.first, cfg));
             if( (src->get_enclosing_function() == func) &&
                 (dst->get_enclosing_function() == func) )
                  f << " " << nameMap.find(src)->second << " -> " << nameMap.find(dst)->second
                    << " [penwidth=1 fontsize=14 fontcolor=\"grey28\" label=\"\"];" << endl;
         }
         f << "}" << endl;                    
      }
};
#endif
