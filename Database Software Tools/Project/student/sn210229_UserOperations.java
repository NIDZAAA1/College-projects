
package student;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.UserOperations;

/**
 *
 * @author nikol
 */
public class sn210229_UserOperations implements UserOperations{

    private static Connection conn = null;
    private static sn210229_UserOperations inst = null;
    
    private sn210229_UserOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_UserOperations getInstance(){
        if (inst == null) inst = new sn210229_UserOperations();
        return inst;
    }
    
    
    @Override
    public boolean insertUser(String userName, String firstName, String lastName, String password) { 
        if (getAllUsers().contains(userName))return false;
        String query = "insert into Korisnik (Ime, Prezime, KorisnickoIme, Sifra, BrPoslatihPaketa)" +
                " values (?,?,?,?,0)";
        if(Character.isLowerCase(firstName.charAt(0)) || Character.isLowerCase(lastName.charAt(0)))return false;
        if(password.length()<8 || !password.matches(".*[a-zA-Z].*") || !password.matches(".*\\d.*"))return false;
        try {
            PreparedStatement ps = conn.prepareStatement(query);
            ps.setString(1, firstName);
            ps.setString(2, lastName);
            ps.setString(3, userName);
            ps.setString(4, password);
            int rez = ps.executeUpdate();
            if(rez != 0)return true;
            return false;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return false;
        }
    }
    public List<String> getAdmins(){
        ArrayList<String> rez = new ArrayList<>();
        String query = "select KorisnickoIme from Administrator";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    
    private int getSentPackagesPerUser(String username){
        int rez=0;
        String query = "select coalesce(BrPoslatihPaketa,0) from Korisnik where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,username);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                rez = rs.getInt(1);
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    @Override
    public Integer getSentPackages(String... strings) { 
        int rez = 0;
        boolean postoji = false;
        for(String s : strings){
            if(getAllUsers().contains(s)){rez+=getSentPackagesPerUser(s);postoji=true;}
        }
        if(postoji)return rez;
        return null;
    }
    
    private int deleteUser(String username){ 
        if(getAllUsers().contains(username)==false)return 0;
        
        String query1 = "delete from Administrator where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query1,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,username);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        String query = "delete from Korisnik where KorisnickoIme = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,username);
            int rez = ps.executeUpdate();
            return rez;
        } catch (Exception e) {e.printStackTrace();}
        return 0;
    }
    
    @Override
    public int declareAdmin(String userName) { 
        if(getAdmins().contains(userName))return 1;
        if(getAllUsers().contains(userName)==false)return 2;
        String query = "insert into Administrator" +
                " values (?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setString(1, userName);
            ps.executeUpdate();
            return 0;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return -1;
        }
    }
    
    @Override
    public int deleteUsers(String... strings) {  
        int rez = 0;
        for (String s : strings) {
            rez+= deleteUser(s);
        };
        return rez;
    }

    @Override
    public List<String> getAllUsers() { 
        ArrayList<String> rez = new ArrayList<>();
        String query = "select KorisnickoIme from Korisnik";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getString(1));
            }
        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
}
