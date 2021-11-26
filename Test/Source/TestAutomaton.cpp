#include "../../Source/Regex/AST/RegexWriter.h"
#include <VlppOS.h>

using namespace vl;
using namespace vl::collections;
using namespace vl::regex_internal;
using namespace vl::stream;

extern WString GetTestResourcePath();
extern WString GetTestOutputPath();

void PrintAutomaton(WString fileName, Automaton::Ref automaton)
{
	FileStream file(GetTestOutputPath() + fileName, FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream output(file, encoder);
	StreamWriter writer(output);

	wchar_t intbuf[100] = { 0 };
	for (vint i = 0; i < automaton->states.Count(); i++)
	{
		State* state = automaton->states[i].Obj();
		if (automaton->startState == state)
		{
			writer.WriteString(L"[START]");
		}
		if (state->finalState)
		{
			writer.WriteString(L"[FINISH]");
		}
		writer.WriteString(L"State<");
		ITOW_S(i, intbuf, sizeof(intbuf) / sizeof(*intbuf), 10);
		writer.WriteString(intbuf);
		writer.WriteLine(L">");
		for (vint j = 0; j < state->transitions.Count(); j++)
		{
			Transition* transition = state->transitions[j];
			ITOW_S(automaton->states.IndexOf(transition->target), intbuf, sizeof(intbuf) / sizeof(*intbuf), 10);
			writer.WriteString(L"    To State<");
			writer.WriteString(intbuf);
			writer.WriteString(L"> : ");
			switch (transition->type)
			{
			case Transition::Chars:
				writer.WriteString(L"<Char : ");
				_itow_s(transition->range.begin, intbuf, sizeof(intbuf) / sizeof(*intbuf), 10);
				writer.WriteString(intbuf);
				writer.WriteString(L"[");
				writer.WriteString(u32tow(U32String::FromChar(transition->range.begin)));
				writer.WriteString(L"] - ");
				_itow_s(transition->range.end, intbuf, sizeof(intbuf) / sizeof(*intbuf), 10);
				writer.WriteString(intbuf);
				writer.WriteString(L"[");
				writer.WriteString(u32tow(U32String::FromChar(transition->range.end)));
				writer.WriteLine(L"]>");
				break;
			case Transition::Epsilon:
				writer.WriteLine(L"<Epsilon>");
				break;
			case Transition::BeginString:
				writer.WriteLine(L"^");
				break;
			case Transition::EndString:
				writer.WriteLine(L"$");
				break;
			case Transition::Nop:
				writer.WriteLine(L"<Nop>");
				break;
			case Transition::Capture:
				writer.WriteString(L"<Capture : ");
				writer.WriteString(u32tow(automaton->captureNames[transition->capture]));
				writer.WriteLine(L" >");
				break;
			case Transition::Match:
				writer.WriteString(L"<Match : ");
				writer.WriteString(u32tow(automaton->captureNames[transition->capture]));
				writer.WriteString(L";");
				ITOW_S(transition->index, intbuf, sizeof(intbuf) / sizeof(*intbuf), 10);
				writer.WriteString(intbuf);
				writer.WriteLine(L" >");
				break;
			case Transition::Positive:
				writer.WriteLine(L"<Positive>");
				break;
			case Transition::Negative:
				writer.WriteLine(L"<Negative>");
				break;
			case Transition::NegativeFail:
				writer.WriteLine(L"<NegativeFail>");
				break;
			case Transition::End:
				writer.WriteLine(L"<End>");
				break;
			}
		}
	}
}

void CompareToBaseline(WString fileName)
{
	WString generatedPath = GetTestOutputPath() + fileName;
	WString baselinePath = GetTestResourcePath() + L"Baseline/" + fileName;

	FileStream generatedFile(generatedPath, FileStream::ReadOnly);
	FileStream baselineFile(baselinePath, FileStream::ReadOnly);

	BomDecoder generatedDecoder;
	BomDecoder baselineDecoder;

	DecoderStream generatedStream(generatedFile, generatedDecoder);
	DecoderStream baselineStream(baselineFile, baselineDecoder);

	StreamReader generatedReader(generatedStream);
	StreamReader baselineReader(baselineStream);

	TEST_CASE(fileName)
	{
		TEST_ASSERT(generatedReader.ReadToEnd() == baselineReader.ReadToEnd());
	});
}

void PrintRegex(bool pure, WString name, U32String code)
{
	TEST_CATEGORY(name + L": " + u32tow(code))
	{
		RegexExpression::Ref regex = ParseRegexExpression(code);
		Expression::Ref expression = regex->Merge();
		CharRange::List subsets;
		expression->NormalizeCharSet(subsets);

		Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
		PrintAutomaton(name + L".eNfa.txt", eNfa);
		CompareToBaseline(name + L".eNfa.txt");

		if (pure)
		{
			Dictionary<State*, State*> nfaStateMap;
			Group<State*, State*> dfaStateMap;
			Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
			Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);

			PrintAutomaton(name + L".pureNfa.txt", nfa);
			PrintAutomaton(name + L".pureDfa.txt", dfa);

			CompareToBaseline(name + L".pureNfa.txt");
			CompareToBaseline(name + L".pureDfa.txt");
		}

		{
			Dictionary<State*, State*> nfaStateMap;
			Group<State*, State*> dfaStateMap;
			Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, RichEpsilonChecker, nfaStateMap);
			Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);

			PrintAutomaton(name + L".richNfa.txt", nfa);
			PrintAutomaton(name + L".richDfa.txt", dfa);

			CompareToBaseline(name + L".richNfa.txt");
			CompareToBaseline(name + L".richDfa.txt");
		}
	});
}

TEST_FILE
{
	TEST_CATEGORY(L"Automaton")
	{
		PrintRegex(true,	L"RegexInteger",		U"/d");
		PrintRegex(true,	L"RegexFullint",		U"(/+|-)?/d+");
		PrintRegex(true,	L"RegexFloat",			U"(/+|-)?/d+(./d+)?");
		PrintRegex(true,	L"RegexString",			UR"("([^\\"]|\\\.)*")");
		PrintRegex(true,	L"RegexComment",		U"///*([^*]|/*+[^*//])*/*+//");
		PrintRegex(false,	L"RegexIP",				U"(<#sec>(<sec>/d+))((<&sec>).){3}(<&sec>)");
		PrintRegex(false,	L"RegexDuplicate",		U"^(<sec>/.+)(<$sec>)+$");
		PrintRegex(false,	L"RegexPrescan",		U"/d+(=/w+)(!vczh)");
	});
}