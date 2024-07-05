
package student;

import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.PackageOperations;

/**
 *
 * @author nikol
 */
public class sn210229_PackageOperations implements PackageOperations{

    public class Par1<Integer,BigDecimal> implements PackageOperations.Pair<Integer, BigDecimal>{
        private Integer first;
        private BigDecimal second;
        @Override
        public Integer getFirstParam() {
            return first;
        }
        @Override
        public BigDecimal getSecondParam() {
            return second;
        }

        public Par1(Integer first, BigDecimal second) {
            this.first = first;
            this.second = second;
        }
    }
    
    private static Connection conn = null;
    private static sn210229_PackageOperations inst = null;
    
    private sn210229_PackageOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_PackageOperations getInstance(){
        if (inst == null) inst = new sn210229_PackageOperations();
        return inst;
    }
    
    void obrisiPonudu(int PonudaId){
        String query = "delete from Ponuda where Id = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, PonudaId);
            int rez = ps.executeUpdate();
        }catch (Exception e) {e.printStackTrace();}
    }
    void obrisiPonudeZaPaket(int packageId){
        String query3 = "select Id from Ponuda where IdPaket = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query3,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,packageId);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                obrisiPonudu(rs.getInt(1));
            }
        } catch (Exception e) {e.printStackTrace();}
    }
    
    

    @Override
    public int insertTransportOffer(String couriersUserName, int packageId, BigDecimal pricePercentage) {
        if(sn210229_PackageOperations.getInstance().getAllPackages().contains(packageId)==false)return -1;
        if(sn210229_CourierOperations.getInstance().getAllCouriers().contains(couriersUserName)==false)return -1;
        String query = "insert into Ponuda (Procenat,IdZ, Kurir)" +
                " values (?, ?, ?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setBigDecimal(1, pricePercentage);
            ps.setInt(2, packageId);
            ps.setString(3, couriersUserName);
            ps.executeUpdate();
            ResultSet rs = ps.getGeneratedKeys();
            if(rs.next()){
                return rs.getInt(1);
            }
            return -1;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return -1;
        }
    }
    
    public double distancaIzmedjuGradova(int IdOpstinaOd,int IdOpstinaDo){
        String query = "select X,Y from Opstina where Id = ?";
        int x1=0;
        int y1=0;
        int x2=0;
        int y2=0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,IdOpstinaOd);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                x1 = rs.getInt(1);
                y1 = rs.getInt(2);
            }
            ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,IdOpstinaDo);
            rs = ps.executeQuery();
            if(rs.next()){
                x2 = rs.getInt(1);
                y2 = rs.getInt(2);
            }
        }catch (Exception e) {e.printStackTrace();}
        return Math.sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    }
    
    public BigDecimal calculatePrice(int packageId, BigDecimal offerPercentage){
        ArrayList<Integer> osnovnaCena = new ArrayList<>();
        osnovnaCena.add(10);
        osnovnaCena.add(25);
        osnovnaCena.add(75);
        ArrayList<Integer> tezinskiFaktor = new ArrayList<>();
        tezinskiFaktor.add(0);
        tezinskiFaktor.add(1);
        tezinskiFaktor.add(2);
        ArrayList<Integer> cenaPoKg = new ArrayList<>();
        cenaPoKg.add(0);
        cenaPoKg.add(100);
        cenaPoKg.add(300);
        String query = "select Tip, Tezina, IdOpstinaOd, IdOpstinaDo from Paket where Id = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                int i = rs.getInt(1);
                BigDecimal procenat = offerPercentage;
                double weight = rs.getBigDecimal(2).doubleValue();
                double euklidskaDistanca = distancaIzmedjuGradova(rs.getInt(3),rs.getInt(4));
                double konacnaCena = (osnovnaCena.get(i) + (tezinskiFaktor.get(i) * weight) * cenaPoKg.get(i))*euklidskaDistanca;
                return BigDecimal.valueOf(konacnaCena).multiply(procenat.divide(new BigDecimal(100)).add(BigDecimal.ONE));
            }
        }catch (Exception e) {e.printStackTrace();}
        return BigDecimal.ZERO;
    }
    
    @Override
    public int insertPackage(int districtFrom, int districtTo, String userName, int packageType, BigDecimal weight) {
        int rez = -1;
        if (districtFrom == districtTo)return -1;
        if(sn210229_DistrictOperations.getInstance().getAllDistricts().contains(districtFrom) == false)return -1;
        if(sn210229_DistrictOperations.getInstance().getAllDistricts().contains(districtTo) == false)return -1;
        if(sn210229_UserOperations.getInstance().getAllUsers().contains(userName)==false)return -1;
        if (packageType<0 || packageType > 3)return -1;
        String query = "insert into Paket (Tip, Tezina, IdOpstinaOd, IdOpstinaDo, Status, Cena, VremePrihvatanja, Kurir, Korisnik)" +
                " values (?,?,?,?,0,0,NULL,NULL,?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setInt(1, packageType);
            ps.setBigDecimal(2, weight);
            ps.setInt(3, districtFrom);
            ps.setInt(4, districtTo);
            ps.setString(5, userName);
            ps.executeUpdate();
            ResultSet rs = ps.getGeneratedKeys();
            if(rs.next()){
                rez = rs.getInt(1);
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
        return rez;
    }
    
    @Override
    public boolean acceptAnOffer(int offerId) {
        int packageId = 0;
        String packageKurir = "";
        String query = "select Paket.Id,Paket.Status,Cena,VremePrihvatanja,Paket.Kurir, Ponuda.Kurir, Ponuda.Procenat from Paket join Ponuda on Paket.Id = Ponuda.IdZ where Ponuda.Id = ?";
        String query1 = "insert into Prevozi (Kurir, IdPaket) values (?,?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,offerId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rs.updateInt(2,1);
                packageId = rs.getInt(1);
                rs.updateBigDecimal(3,calculatePrice(rs.getInt(1),rs.getBigDecimal(7)));
                rs.updateDate(4, Date.valueOf(LocalDate.now()));
                packageKurir = rs.getString(6);
                rs.updateString(5,packageKurir);
                rs.updateRow();
            }
            ps = conn.prepareStatement(query1,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,packageKurir);
            ps.setInt(2,packageId);
            int rez =  ps.executeUpdate();
            if(rez != 0){
                return true;
            }
            return false;
        }catch (Exception e) {e.printStackTrace();return false;}
    }

    @Override
    public List<Integer> getAllOffers() {
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Ponuda";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    
    @Override
    public boolean deletePackage(int packageId) { 
        if(getAllPackages().contains(packageId)==false)return false;
        String query = "delete from Paket where Id = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,packageId);
            int rez = ps.executeUpdate();
            if(rez != 0)return true;
            return false;
             
        } catch (Exception e) {e.printStackTrace();}
        return false;
    }

    @Override
    public boolean changeWeight(int packageId, BigDecimal newWeight) {
        if(getDeliveryStatus(packageId)!=0)return false;
        if(getAllPackages().contains(packageId)==false)return false;
        String query = "select Tezina from Paket where Id = ?";
        boolean uspeh = false;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rs.updateBigDecimal(1, newWeight);
                rs.updateRow();
                uspeh = true;
            }else{
                uspeh = false;
            }
             
        } catch (Exception e) {e.printStackTrace();}
        return uspeh;
    }

    @Override
    public List<PackageOperations.Pair<Integer, BigDecimal>> getAllOffersForPackage(int packageId) {
        ArrayList<PackageOperations.Pair<Integer,BigDecimal>> rez = new ArrayList<>();
        String query = "select Id, Procenat from Ponuda where IdZ = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                Par1 noviPar = new Par1(rs.getInt(1),rs.getBigDecimal(2));
                rez.add(noviPar);
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    @Override
    public boolean changeType(int packageId, int newType) {
        if(getDeliveryStatus(packageId)!=0)return false;
        if(getAllPackages().contains(packageId)==false)return false;
        String query = "select Tip from Paket where Id = ?";
        boolean uspeh = false;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rs.updateInt(1, newType);
                rs.updateRow();
                uspeh = true;
            }else{
                uspeh = false;
            }
             
        } catch (Exception e) {e.printStackTrace();}
        return uspeh;
    }

    @Override
    public Integer getDeliveryStatus(int packageId) {
        if(getAllPackages().contains(packageId) == false)return null;
        String query = "select Status from Paket where Id = ?";
        int status = 0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                status = rs.getInt(1);
            }
             
        } catch (Exception e) {e.printStackTrace();}
        return status;
    }

    @Override
    public Date getAcceptanceTime(int packageId) {
        if(getAllPackages().contains(packageId) == false)return null;
        String query = "select VremePrihvatanja from Paket where Id = ?";
        int status = 0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                Date vreme = rs.getDate(1);
                return vreme;
            }
            return null;
        } catch (Exception e) {e.printStackTrace();}
        return null;
    }

    @Override
    public List<Integer> getAllPackagesWithSpecificType(int type) {
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Paket where Tip = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,type);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    @Override
    public BigDecimal getPriceOfDelivery(int packageId) {
        if(getAllPackages().contains(packageId) == false)return null;
        String query = "select Status,Cena from Paket where Id = ?";
        int status = 0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                status = rs.getInt(1);
                if(status==0)return null;
                BigDecimal cena = rs.getBigDecimal(2);
                return cena;
            }
        } catch (Exception e) {e.printStackTrace();}
        return null;
    }
    
    @Override
    public List<Integer> getAllPackages() {
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Paket";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    @Override
    public List<Integer> getDrive(String courierUsername) {
       ArrayList<Integer> rez = new ArrayList<>();
       String query = "select coalesce(count(*),0) from Prevozi where Kurir = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUsername);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                if(rs.getInt(1)==0){
                    return null;
                }
            }
        }catch (Exception e) {e.printStackTrace();}
        query = "select IdPaket from Prevozi join Paket on Prevozi.IdPaket = Paket.Id where Prevozi.Kurir = ? and ( Paket.Status = ? or Paket.Status = ?)order by VremePrihvatanja ASC";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUsername);
            ps.setInt(2,1);
            ps.setInt(3,2);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    @Override
    public int driveNextPackage(String courierUsername) {
        if(getDrive(courierUsername).isEmpty())return -1;
        if(sn210229_CourierOperations.getInstance().getCouriersWithStatus(0).contains(courierUsername)){
            sn210229_CourierOperations.getInstance().updateStatus(courierUsername, 1);
        }
        int packageId = getDrive(courierUsername).get(0);
        String query = "update Paket set Status = 3 where Id = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, packageId);
            int rez = ps.executeUpdate();
        }catch (Exception e) {e.printStackTrace();return -2;}
        if(getDrive(courierUsername).size()>0){
            int packageId2 = getDrive(courierUsername).get(0);
            String query2 = "update Paket set Status = 2 where Id = ?";
            try {
                PreparedStatement ps = conn.prepareStatement(query2,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
                ps.setInt(1, packageId2);
                int rez = ps.executeUpdate();
            }catch (Exception e) {e.printStackTrace();return -2;}
        }
        query = "update Kurir set BrIsporucenihPaketa = BrIsporucenihPaketa + 1 where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUsername);
            int rez = ps.executeUpdate();
        }catch (Exception e) {e.printStackTrace();return -2;}
        
        if(getDrive(courierUsername).isEmpty()){
            ArrayList<Integer> ceneGoriva = new ArrayList<>();
            ceneGoriva.add(15);
            ceneGoriva.add(32);
            ceneGoriva.add(36);
            sn210229_CourierOperations.getInstance().updateStatus(courierUsername, 0);
            BigDecimal profit = new BigDecimal(0.0);
            BigDecimal fuelCost = new BigDecimal(0.0);
            boolean prvi = true;
            double distance = 0;
            int prethodniGrad = 0;
            String query2 = "select Paket.Id, Paket.Cena, Paket.IdOpstinaOd, Paket.IdOpstinaDo, Vozilo.TipGoriva, Vozilo.Potrosnja from Prevozi join Paket on Prevozi.IdPaket = Paket.Id join Kurir on Kurir.KorisnickoIme = Prevozi.Kurir join Vozilo on Vozilo.RegistracioniBroj = Kurir.RegistracioniBroj where Prevozi.Kurir = ? order by VremePrihvatanja ASC";
            try {
                PreparedStatement ps = conn.prepareStatement(query2,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
                ps.setString(1, courierUsername);
                ResultSet rs = ps.executeQuery();
                while(rs.next()){
                    profit = profit.add(rs.getBigDecimal(2));
                    int idOpstinaOd = rs.getInt(3);
                    int idOpstinaDo = rs.getInt(4);
                    int tipGoriva = rs.getInt(5);
                    BigDecimal potrosnja = rs.getBigDecimal(6);
                    if(prvi == false){
                        distance = distancaIzmedjuGradova(prethodniGrad, idOpstinaOd);
                        fuelCost = fuelCost.add(potrosnja.multiply(new BigDecimal(ceneGoriva.get(tipGoriva))).multiply(new BigDecimal(distance)));
                    }
                    distance = distancaIzmedjuGradova(idOpstinaOd, idOpstinaDo);
                    fuelCost = fuelCost.add(potrosnja.multiply(new BigDecimal(ceneGoriva.get(tipGoriva))).multiply(new BigDecimal(distance)));
                    
                    prvi = false;
                    prethodniGrad = idOpstinaDo;
                }
                profit = profit.subtract(fuelCost);
            }catch (Exception e) {e.printStackTrace();return -2;}
            sn210229_CourierOperations.getInstance().addProfit(courierUsername, profit);
            
            String query1 = "delete from Prevozi where Kurir = ?";
            try {
                PreparedStatement ps = conn.prepareStatement(query1,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
                ps.setString(1,courierUsername);
                ps.executeUpdate();
            } catch (Exception e) {e.printStackTrace();return -2;}
        }
        return packageId;
    }
    
}
