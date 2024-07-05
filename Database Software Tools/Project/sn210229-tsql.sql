

CREATE PROCEDURE SP_Procedura
	@Username varchar(100),
	@RegistracioniBroj varchar(100),
	@rezultat int output
AS
BEGIN
	declare @greska1 int, @greska2 int
	set @rezultat = -1
	select @greska1 = coalesce(count(*),0) from Kurir where RegistracioniBroj = @RegistracioniBroj or KorisnickoIme = @Username
	if(@greska1>0)return

	insert into Kurir values (0, 0.0, 0, @RegistracioniBroj, @Username)
END
GO



CREATE TRIGGER TR_TransportOffer_TrigerPaket 
   ON  Paket
   AFTER UPDATE
AS 
BEGIN
	Declare @noviStatus int
	Declare @stariStatus int
	Declare @paketId int
	Declare @MyCursorI Cursor
	Declare @MyCursorD Cursor

	SET @MyCursorD = CURSOR FOR
	select Status
	from deleted

	SET @MyCursorI = CURSOR FOR
	select Status, Id
	from inserted

	OPEN @MyCursorI
	OPEN @MyCursorD

	FETCH NEXT FROM @MyCursorD
	INTO @stariStatus
	FETCH NEXT FROM @MyCursorI
	INTO @noviStatus, @paketId

	while @@FETCH_STATUS = 0
	begin
	if(@noviStatus = 1 and @stariStatus = 0)
	begin
		delete from Ponuda where IdZ= @paketId
	end
	FETCH NEXT FROM @MyCursorD
	INTO @stariStatus
	FETCH NEXT FROM @MyCursorI
	INTO @noviStatus, @paketId
	end
END
GO
