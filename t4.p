program boo(input, output);
    var x: integer;

    procedure print(j: integer);
    
        function next(k: integer): integer;
        begin
            next := x + 1
        end;

    begin
        j := next(j)
    end;

begin
    x := 1;
    while x <= 10 do print(x)
end.
