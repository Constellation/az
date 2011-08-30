// CFA2 based JS completion implementation
//
// CFA2 original report is
//   http://www.ccs.neu.edu/home/dimvar/papers/cfa2-NU-CCIS-10-01.pdf
// and highly inspired from doctorjs src code
//   http://doctorjs.org/
//
//
// following is Doctor.JS license
//
// in Az, newly created source code is under New BSD License
// and, Doctor.JS source code is under MPL 1.1/GPL 2.0/LGPL 2.1
//
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an 'AS IS' basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Bespin.
 *
 * The Initial Developer of the Original Code is
 * Dimitris Vardoulakis <dimvar@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dimitris Vardoulakis <dimvar@gmail.com>
 *   Patrick Walton <pcwalton@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef AZ_CFA2_H_
#define AZ_CFA2_H_
#include <az/cfa2/completer.h>
#include <az/cfa2/cli_completer.h>
#include <az/cfa2/heap.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/binding_resolver.h>
#include <az/cfa2/interpreter.h>
namespace az {
namespace cfa2 {

template<typename Source, typename Reporter>
inline void Complete(FunctionLiteral* global,
                     Heap* heap,
                     const Source& src,
                     Reporter* reporter) {
  {
    // resolve binding type
    BindingResolver resolver(heap);
    resolver.Resolve(global);
  }
  {
    // initialize heap
    //
    // initialize summaries and heap static objects declaration
    // static objects are bound to heap by AstNode address and
    // summaries are bound to heap by FunctionLiteral address
    HeapInitializer initializer(heap);
    initializer.Initialize(global);
  }
  if (heap->completer() &&
      !heap->IsWaited(heap->completer()->GetTargetFunction())) {
    return;
  }
  {
    // execute abstract interpreter
    Interpreter interp(heap);
    interp.Run(global);
  }
  // heap->ShowSummaries();  // for debug...
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_H_
