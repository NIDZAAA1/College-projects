
package student;

import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.VehicleOperations;

/**
 *
 * @author nikol
 */
public class sn210229_VehicleOperations implements VehicleOperations{

    private static Connection conn = null;
    private static sn210229_VehicleOperations inst = null;
    
    private sn210229_VehicleOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_VehicleOperations getInstance(){
        if (inst == null) inst = new sn210229_VehicleOperations();
        return inst;
    }

    private int deleteVehicle(String licensePlateNumber){
        if(getAllVehichles().contains(licensePlateNumber)==false)return 0;
        String query = "delete from Vozilo where RegistracioniBroj = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, licensePlateNumber);
            ps.executeUpdate();
            return 1;
        }catch (Exception e) {e.printStackTrace();return 0;}
    }
    
    public List<String> getTakenVehicles() {
        ArrayList<String> rez = new ArrayList<>();
        String query = "select Vozilo.RegistracioniBroj from Vozilo join Kurir on Kurir.RegistracioniBroj = Vozilo.RegistracioniBroj";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    @Override
    public int deleteVehicles(String... strings) { 
        int rez = 0;
        for (String s : strings) {
            rez+= deleteVehicle(s);
        };
        return rez;
    }

    @Override
    public List<String> getAllVehichles() { 
       ArrayList<String> rez = new ArrayList<>();
        String query = "select RegistracioniBroj from Vozilo";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }

    @Override
    public boolean changeFuelType(String licensePlateNumber, int fuelType) { 
        ArrayList<String> rez = new ArrayList<>();
        String query = "select TipGoriva from Vozilo where RegistracioniBroj = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, licensePlateNumber);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rs.updateInt(1,fuelType);
                rs.updateRow();
                return true;
            }
        }catch (Exception e) {e.printStackTrace();return false;}
        return false;
    }
    
    @Override
    public boolean insertVehicle(String licencePlateNumber, int fuelType, BigDecimal fuelConsumption) { 
        if (getAllVehichles().contains(licencePlateNumber))return false;
        String query = "insert into Vozilo (RegistracioniBroj, TipGoriva, Potrosnja)" +
                " values (?,?,?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query);
            ps.setString(1, licencePlateNumber);
            ps.setInt(2, fuelType);
            ps.setBigDecimal(3, fuelConsumption);
            int rez = ps.executeUpdate();
            if(rez != 0)return true;
            return false;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return false;
        }
    }

    @Override
    public boolean changeConsumption(String licensePlateNumber, BigDecimal fuelConsumption) { 
        ArrayList<String> rez = new ArrayList<>();
        String query = "select Potrosnja from Vozilo where RegistracioniBroj = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1, licensePlateNumber);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rs.updateBigDecimal(1,fuelConsumption);
                rs.updateRow();
                return true;
            }
        }catch (Exception e) {e.printStackTrace();return false;}
        return false;
    }
    
}
