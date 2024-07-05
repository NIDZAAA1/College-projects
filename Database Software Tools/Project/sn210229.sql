
CREATE TABLE [Administrator]
( 
	[KorisnickoIme]      varchar(100)  NOT NULL 
)
go

CREATE TABLE [Grad]
( 
	[Id]                 integer  IDENTITY  NOT NULL ,
	[Naziv]              varchar(100)  NULL ,
	[PostanskiBroj]      varchar(100)  NULL 
)
go

CREATE TABLE [Korisnik]
( 
	[Ime]                varchar(100)  NULL ,
	[Prezime]            varchar(100)  NULL ,
	[KorisnickoIme]      varchar(100)  NOT NULL ,
	[Sifra]              varchar(100)  NULL ,
	[BrPoslatihPaketa]   integer  NULL 
)
go

CREATE TABLE [Kurir]
( 
	[BrIsporucenihPaketa] integer  NULL ,
	[Profit]             decimal(10,3)  NULL ,
	[Status]             integer  NULL 
	CONSTRAINT [KurirDefaultStatus_468536504]
		 DEFAULT  0
	CONSTRAINT [KurirStatus]
		CHECK  ( [Status]=0 OR [Status]=1 ),
	[RegistracioniBroj]  varchar(100)  NULL ,
	[KorisnickoIme]      varchar(100)  NOT NULL 
)
go

CREATE TABLE [Opstina]
( 
	[Id]                 integer  IDENTITY  NOT NULL ,
	[Naziv]              varchar(100)  NULL ,
	[X]                  integer  NULL ,
	[Y]                  integer  NULL ,
	[IdG]                integer  NOT NULL 
)
go

CREATE TABLE [Paket]
( 
	[Id]                 integer  IDENTITY  NOT NULL ,
	[Tip]                integer  NULL 
	CONSTRAINT [PaketDefaultTip_373242498]
		 DEFAULT  0
	CONSTRAINT [PaketTip]
		CHECK  ( [Tip]=0 OR [Tip]=1 OR [Tip]=2 ),
	[Tezina]             decimal(10,3)  NULL ,
	[IdOpstinaOd]        integer  NOT NULL ,
	[IdOpstinaDo]        integer  NOT NULL ,
	[Status]             integer  NULL 
	CONSTRAINT [PaketDefaultStatus_922241477]
		 DEFAULT  0
	CONSTRAINT [PaketStatus]
		CHECK  ( [Status]=0 OR [Status]=1 OR [Status]=2 OR [Status]=3 ),
	[Cena]               decimal(10,3)  NULL ,
	[VremePrihvatanja]   DATETIME  NULL ,
	[Kurir]              varchar(100)  NULL ,
	[Korisnik]           varchar(100)  NOT NULL 
)
go

CREATE TABLE [Ponuda]
( 
	[Id]                 integer  IDENTITY  NOT NULL ,
	[Procenat]           decimal(10,3)  NULL ,
	[IdZ]                integer  NULL ,
	[Kurir]              varchar(100)  NOT NULL 
)
go

CREATE TABLE [Prevozi]
( 
	[Id]                 integer  IDENTITY  NOT NULL ,
	[Kurir]              varchar(100)  NOT NULL ,
	[IdPaket]            integer  NOT NULL 
)
go

CREATE TABLE [Vozilo]
( 
	[RegistracioniBroj]  varchar(100)  NOT NULL ,
	[TipGoriva]          integer  NULL 
	CONSTRAINT [VoziloDefaultGorivo_1296865967]
		 DEFAULT  0
	CONSTRAINT [VoziloTipGoriva]
		CHECK  ( [TipGoriva]=0 OR [TipGoriva]=1 OR [TipGoriva]=2 ),
	[Potrosnja]          decimal(10,3)  NULL 
)
go

ALTER TABLE [Administrator]
	ADD CONSTRAINT [XPKAdministrator] PRIMARY KEY  CLUSTERED ([KorisnickoIme] ASC)
go

ALTER TABLE [Grad]
	ADD CONSTRAINT [XPKGrad] PRIMARY KEY  CLUSTERED ([Id] ASC)
go

ALTER TABLE [Korisnik]
	ADD CONSTRAINT [XPKKorisnik] PRIMARY KEY  CLUSTERED ([KorisnickoIme] ASC)
go

ALTER TABLE [Kurir]
	ADD CONSTRAINT [XPKKurir] PRIMARY KEY  CLUSTERED ([KorisnickoIme] ASC)
go

ALTER TABLE [Opstina]
	ADD CONSTRAINT [XPKOpstina] PRIMARY KEY  CLUSTERED ([Id] ASC)
go

ALTER TABLE [Paket]
	ADD CONSTRAINT [XPKPaket] PRIMARY KEY  CLUSTERED ([Id] ASC)
go

ALTER TABLE [Ponuda]
	ADD CONSTRAINT [XPKPonuda] PRIMARY KEY  CLUSTERED ([Id] ASC)
go

ALTER TABLE [Prevozi]
	ADD CONSTRAINT [XPKPrevozi] PRIMARY KEY  CLUSTERED ([Id] ASC)
go

ALTER TABLE [Vozilo]
	ADD CONSTRAINT [XPKVozilo] PRIMARY KEY  CLUSTERED ([RegistracioniBroj] ASC)
go

ALTER TABLE [Vozilo]
	ADD CONSTRAINT [RegistracijaUnique] UNIQUE ([RegistracioniBroj]  ASC)
go


ALTER TABLE [Administrator]
	ADD CONSTRAINT [R_5] FOREIGN KEY ([KorisnickoIme]) REFERENCES [Korisnik]([KorisnickoIme])
		ON DELETE CASCADE
		ON UPDATE CASCADE
go


ALTER TABLE [Kurir]
	ADD CONSTRAINT [R_7] FOREIGN KEY ([RegistracioniBroj]) REFERENCES [Vozilo]([RegistracioniBroj])
		ON DELETE NO ACTION
		ON UPDATE CASCADE
go

ALTER TABLE [Kurir]
	ADD CONSTRAINT [R_6] FOREIGN KEY ([KorisnickoIme]) REFERENCES [Korisnik]([KorisnickoIme])
		ON DELETE CASCADE
		ON UPDATE CASCADE
go


ALTER TABLE [Opstina]
	ADD CONSTRAINT [R_4] FOREIGN KEY ([IdG]) REFERENCES [Grad]([Id])
		ON DELETE NO ACTION
		ON UPDATE CASCADE
go


ALTER TABLE [Paket]
	ADD CONSTRAINT [R_8] FOREIGN KEY ([IdOpstinaOd]) REFERENCES [Opstina]([Id])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go

ALTER TABLE [Paket]
	ADD CONSTRAINT [R_9] FOREIGN KEY ([IdOpstinaDo]) REFERENCES [Opstina]([Id])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go

ALTER TABLE [Paket]
	ADD CONSTRAINT [R_10] FOREIGN KEY ([Korisnik]) REFERENCES [Korisnik]([KorisnickoIme])
		ON DELETE NO ACTION
		ON UPDATE CASCADE
go

ALTER TABLE [Paket]
	ADD CONSTRAINT [R_12] FOREIGN KEY ([Kurir]) REFERENCES [Kurir]([KorisnickoIme])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go


ALTER TABLE [Ponuda]
	ADD CONSTRAINT [R_11] FOREIGN KEY ([IdZ]) REFERENCES [Paket]([Id])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go

ALTER TABLE [Ponuda]
	ADD CONSTRAINT [R_13] FOREIGN KEY ([Kurir]) REFERENCES [Kurir]([KorisnickoIme])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go


ALTER TABLE [Prevozi]
	ADD CONSTRAINT [R_16] FOREIGN KEY ([Kurir]) REFERENCES [Kurir]([KorisnickoIme])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go

ALTER TABLE [Prevozi]
	ADD CONSTRAINT [R_17] FOREIGN KEY ([IdPaket]) REFERENCES [Paket]([Id])
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
go



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
