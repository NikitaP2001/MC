int run_pp_lexer();
int run_pp_parser();
int run_pp_node();

int main()
{
        int result = 0;
        result |= run_pp_lexer();
        result |= run_pp_parser();
        result |= run_pp_node();
        return result;
}