
package student;

import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.CourierOperations;

/**
 *
 * @author nikol
 */
public class sn210229_CourierOperations implements CourierOperations {

    private static Connection conn = null;
    private static sn210229_CourierOperations inst = null;
    
    private sn210229_CourierOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_CourierOperations getInstance(){
        if (inst == null) inst = new sn210229_CourierOperations();
        return inst;
    }
    
    

    @Override
    public boolean deleteCourier(String courierUserName) { 
        if(getAllCouriers().contains(courierUserName)==false)return false;
        String query = "delete from Kurir where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUserName);
            int rez = ps.executeUpdate();
            if(rez !=0){
                return true;
            }else{
                return false;
            }
        }catch (Exception e) {e.printStackTrace();return false;}
    }

    @Override
    public List<String> getCouriersWithStatus(int i) { 
        ArrayList<String> rez = new ArrayList<>();
        String query = "select KorisnickoIme from Kurir where status = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, i);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    @Override
    public List<String> getAllCouriers() { 
        ArrayList<String> rez = new ArrayList<>();
        String query = "select KorisnickoIme from Kurir order by Profit DESC";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    
    
    public void updateStatus(String courierUserName, int newStatus){
        String query = "update Kurir set Status = ? where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, newStatus);
            ps.setString(2, courierUserName);
            int rez = ps.executeUpdate();
        }catch (Exception e) {e.printStackTrace();}
    }
    
    public void addProfit(String courierUserName, BigDecimal profit){
        String query = "select Profit from Kurir where KorisnickoIme = ?";
        BigDecimal stariProfit = BigDecimal.ZERO;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUserName);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                stariProfit = rs.getBigDecimal(1);
            }
        }catch (Exception e) {e.printStackTrace();}
        
        query = "update Kurir set Profit = ? where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setBigDecimal(1, stariProfit.add(profit));
            ps.setString(2, courierUserName);
            int rez = ps.executeUpdate();
        }catch (Exception e) {e.printStackTrace();}
    }
    
    public BigDecimal getKurirPotrosnja(String courierUserName){
        String query = "select Vozilo.Potrosnja from Kurir join Vozilo on Kurir.RegistracioniBroj = Vozilo.RegistracioniBroj where Kurir.KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, courierUserName);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                return rs.getBigDecimal(1);
            }
        }catch (Exception e) {e.printStackTrace();}
        return BigDecimal.ZERO;
    }
    
    @Override
    public boolean insertCourier(String courierUserName, String licencePlateNumber){ 
        if (getAllCouriers().contains(courierUserName))return false;
        if (sn210229_VehicleOperations.getInstance().getAllVehichles().contains(licencePlateNumber) == false)return false;
        if (sn210229_VehicleOperations.getInstance().getTakenVehicles().contains(licencePlateNumber))return false;
        String query = "insert into Kurir (BrIsporucenihPaketa, Profit, Status, KorisnickoIme, RegistracioniBroj)" +
                " values (0,0,0,?,?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setString(1, courierUserName);
            ps.setString(2, licencePlateNumber);
            ps.executeUpdate();
            ResultSet rs = ps.getGeneratedKeys();
            if(rs.next()){
                return true;
            }
            return false;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return false;
        }
    }
    
    @Override
    public BigDecimal getAverageCourierProfit(int i) { 
        String query = "select AVG(Profit) from Kurir where BrIsporucenihPaketa >= ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1, i);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                return rs.getBigDecimal(1);
            }
            return BigDecimal.ZERO;
        }catch (Exception e) {e.printStackTrace();}
        return BigDecimal.ZERO;
    }
}
