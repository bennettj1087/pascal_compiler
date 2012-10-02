program boo(input, output);
    var x: integer;

    procedure print(j, q, s: integer; z, x, y: real);
    
        function next(k: integer): integer;
        begin
            k := j + k;
			print := k + j
        end;

    begin
        j := next(j)
    end;

begin
    x := 1;
    while x < 10 do
		begin
			print(x, 4, 5, 2.5, 3.5, 4.5);
			x := x + 1
		end
end.
