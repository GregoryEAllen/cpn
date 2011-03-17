//===========================================================================
//  Implementing Kahn's simple example in CPN
//===========================================================================

#include "Kernel.h"
#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <stdio.h>
#include <stdlib.h>

using namespace CPN;

//---------------------------------------------------------------------------
//  3 functions f, g, & h as specified in Kahn's simple example
void Kahns_example_f(IQueue<int> in0, IQueue<int> in1, OQueue<int> out)
{
    int value;
    int loop=0;
    while (true) {
        in0.Dequeue(&value, 1);
        out.Enqueue(&value, 1);
        printf("%d",value);
        in1.Dequeue(&value, 1);
        out.Enqueue(&value, 1);
        printf("%d",value);
        
        // periodically print a newline
        if (!(++loop%36)) printf("\n");
    }
}

void Kahns_example_g(IQueue<int> in, OQueue<int> out0, OQueue<int> out1)
{
    int value;
    while (true) {
        in.Dequeue(&value, 1);
        out0.Enqueue(&value, 1);
        in.Dequeue(&value, 1);
        out1.Enqueue(&value, 1);
    }
}

void Kahns_example_h(IQueue<int> in, OQueue<int> out, int first)
{
    int value = first;
    out.Enqueue(&value, 1);
    while (true) {
        in.Dequeue(&value, 1);
        out.Enqueue(&value, 1);
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  create 3 CPN nodes that call Kahn's 3 functions
CPN_DECLARE_NODE_AND_FACTORY(node_f, node_f);
void node_f::Process()
{
    IQueue<int> in0 = GetIQueue("in0");
    IQueue<int> in1 = GetIQueue("in1");
    OQueue<int> out = GetOQueue("out");
    Kahns_example_f(in0,in1,out);
}

CPN_DECLARE_NODE_AND_FACTORY(node_g, node_g);
void node_g::Process()
{
    IQueue<int> in = GetIQueue("in");
    OQueue<int> out0 = GetOQueue("out0");
    OQueue<int> out1 = GetOQueue("out1");
    Kahns_example_g(in,out0,out1);
}

CPN_DECLARE_NODE_AND_FACTORY(node_h, node_h);
void node_h::Process()
{
    IQueue<int> in = GetIQueue("in");
    OQueue<int> out = GetOQueue("out");
    int first = GetParam<int>("first");
    Kahns_example_h(in,out,first);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// main for Kahn's simple example using the CPN Nodes
int main_with_nodes(int argc, const char* argv[])
{

    Kernel kernel("kernel");

    kernel.CreateNode("f", "node_f");
    kernel.CreateNode("g", "node_g");
    kernel.CreateNode( NodeAttr("h0","node_h").SetParam("first", 0) );
    kernel.CreateNode( NodeAttr("h1","node_h").SetParam("first", 1) );

    QueueAttr qdef(2*sizeof(int), sizeof(int));
    qdef.SetDatatype<int>();

    QueueAttr X(qdef), S(qdef), Y(qdef), T(qdef), Z(qdef);
    X.SetWriter( "f", "out").SetReader( "g", "in");
    S.SetWriter( "g","out0").SetReader("h0", "in");
    Y.SetWriter("h0", "out").SetReader( "f","in0");
    T.SetWriter( "g","out1").SetReader("h1", "in");
    Z.SetWriter("h1", "out").SetReader( "f","in1");
    
    kernel.CreateQueue(X);
    kernel.CreateQueue(S);
    kernel.CreateQueue(Y);
    kernel.CreateQueue(T);
    kernel.CreateQueue(Z);

    kernel.WaitForAllNodes();
    
    return 0;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  create 3 functions for CPN function nodes, calling Kahn's 3 functions
void func_f(NodeBase* node)
{
    IQueue<int> in0 = node->GetIQueue("in0");
    IQueue<int> in1 = node->GetIQueue("in1");
    OQueue<int> out = node->GetOQueue("out");
    Kahns_example_f(in0,in1,out);
}

void func_g(NodeBase* node)
{
    IQueue<int> in = node->GetIQueue("in");
    OQueue<int> out0 = node->GetOQueue("out0");
    OQueue<int> out1 = node->GetOQueue("out1");
    Kahns_example_g(in,out0,out1);
}

void func_h(NodeBase* node, int first)
{
    IQueue<int> in = node->GetIQueue("in");
    OQueue<int> out = node->GetOQueue("out");
    Kahns_example_h(in,out,first);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//  main for Kahn's simple example using CPN function nodes
int main_with_funcs(int argc, const char* argv[])
{
    Kernel kernel("kernel");

    kernel.CreateFunctionNode("f", func_f);
    kernel.CreateFunctionNode("g", func_g);
    kernel.CreateFunctionNode("h0", func_h, 0);
    kernel.CreateFunctionNode("h1", func_h, 1);

    QueueAttr qdef(2*sizeof(int), sizeof(int));
    qdef.SetDatatype<int>();

    QueueAttr X(qdef), S(qdef), Y(qdef), T(qdef), Z(qdef);
    X.SetWriter( "f", "out").SetReader( "g", "in");
    S.SetWriter( "g","out0").SetReader("h0", "in");
    Y.SetWriter("h0", "out").SetReader( "f","in0");
    T.SetWriter( "g","out1").SetReader("h1", "in");
    Z.SetWriter("h1", "out").SetReader( "f","in1");

    kernel.CreateQueue(X);
    kernel.CreateQueue(S);
    kernel.CreateQueue(Y);
    kernel.CreateQueue(T);
    kernel.CreateQueue(Z);

    kernel.WaitForAllNodes();

    return 0;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  choose a main....
int main(int argc, const char* argv[])
{
    int which = 1;

    if (argc>1)
        which = atoi(argv[1]);
    
    if (which==1)
        return main_with_nodes(argc,argv);
    if (which==2)
        return main_with_funcs(argc,argv);
    
    printf("usage: %s [which]\n", argv[0]);
    printf("  1 - main_with_nodes\n");
    printf("  2 - main_with_funcs\n");
    return -1;
}
